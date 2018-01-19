#pragma once

#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

#include <zmq.hpp>

#include "exceptions.hpp"
#include "packets.hpp"
#include "packets/geometry_packets.hpp"
#include "packets/reconstruction_packets.hpp"
#include "packets/scene_management_packets.hpp"

namespace tomop {

class server {
  public:
    using callback_type =
        std::function<std::pair<std::array<int32_t, 2>, std::vector<float>>(
            std::array<float, 9>, int32_t)>;

    using projection_callback_type = std::function<void(
        std::array<int32_t, 2>, std::vector<float>, int32_t)>;

    server(std::string name, std::string hostname = "tcp://localhost:5555",
           std::string subscribe_hostname = "tcp://localhost:5556")
        : context_(1), socket_(context_, ZMQ_REQ),
          subscribe_socket_(context_, ZMQ_SUB) {
        using namespace std::chrono_literals;

        // set socket timeout to 200 ms
        socket_.setsockopt(ZMQ_LINGER, 200);
        socket_.connect(hostname);

        auto packet = MakeScenePacket(name, 3);

        packet.send(socket_);

        // check if we get a reply within a second
        zmq::pollitem_t items[] = {{socket_, 0, ZMQ_POLLIN, 0}};
        auto poll_result = zmq::poll(items, 1, 1000ms);

        if (poll_result <= 0) {
            throw server_error("Could not connect to server");
        } else {
            //  get the reply.
            zmq::message_t reply;
            socket_.recv(&reply);
            scene_id_ = *(int32_t*)reply.data();
        }

        subscribe(subscribe_hostname);
    }

    server(int32_t scene_id, int group_size = 1,
           std::string hostname = "tcp://localhost:5555",
           std::string subscribe_hostname = "tcp://localhost:5556")
        : context_(1), socket_(context_, ZMQ_REQ),
          subscribe_socket_(context_, ZMQ_SUB), scene_id_(scene_id) {
        using namespace std::chrono_literals;

        // set socket timeout to 200 ms
        socket_.setsockopt(ZMQ_LINGER, 200);
        socket_.connect(hostname);

        subscribe(subscribe_hostname);

        // request current slices
        auto packet = GroupRequestSlicesPacket(scene_id_, group_size);
        packet.send(socket_);

        // check if we get a reply within a second
        zmq::pollitem_t items[] = {{socket_, 0, ZMQ_POLLIN, 0}};
        auto poll_result = zmq::poll(items, 1, 1000ms);

        if (poll_result <= 0) {
            throw server_error("Could not connect to server");
        } else {
            //  get the reply.
            zmq::message_t reply;
            socket_.recv(&reply);
        }
    }

    ~server() {
        socket_.close();
        subscribe_socket_.close();
        context_.close();

        if (serve_thread_.joinable()) {
            serve_thread_.join();
        }
    }

    void send(const Packet& packet) {
        packet.send(socket_);
        zmq::message_t reply;
        socket_.recv(&reply);
    }

    void subscribe(std::string subscribe_host) {
        if (scene_id_ < 0) {
            throw server_error("Subscribe called for uninitialized server");
        }

        // set socket timeout to 200 ms
        socket_.setsockopt(ZMQ_LINGER, 200);

        //  Socket to talk to server
        subscribe_socket_.connect(subscribe_host);

        std::vector<packet_desc> descriptors = {packet_desc::set_slice,
                                                packet_desc::remove_slice,
                                                packet_desc::kill_scene};

        for (auto descriptor : descriptors) {
            int32_t filter[] = {
                (std::underlying_type<packet_desc>::type)descriptor, scene_id_};
            subscribe_socket_.setsockopt(ZMQ_SUBSCRIBE, filter,
                                         sizeof(decltype(filter)));
        }
    }

    void serve() {
        // not only serve this, but also other..
        auto recast_thread = std::thread([&] {
            while (true) {
                zmq::message_t update;
                bool kill = false;
                if (!subscribe_socket_.recv(&update)) {
                    kill = true;
                } else {
                    auto desc = ((packet_desc*)update.data())[0];
                    auto buffer =
                        memory_buffer(update.size(), (char*)update.data());

                    switch (desc) {
                    case packet_desc::kill_scene: {
                        auto packet = std::make_unique<KillScenePacket>();
                        packet->deserialize(std::move(buffer));

                        if (packet->scene_id != scene_id_) {
                            std::cout << "Received kill request with wrong "
                                         "scene id\n";
                        } else {
                            kill = true;
                        }
                        break;
                    }

                    case packet_desc::set_slice: {
                        auto packet = std::make_unique<SetSlicePacket>();
                        packet->deserialize(std::move(buffer));

                        make_slice(packet->slice_id, packet->orientation);
                        break;
                    }
                    case packet_desc::remove_slice: {
                        auto packet = std::make_unique<RemoveSlicePacket>();
                        packet->deserialize(std::move(buffer));

                        auto to_erase = std::find_if(
                            slices_.begin(), slices_.end(), [&](auto x) {
                                return x.first == packet->slice_id;
                            });
                        slices_.erase(to_erase);
                        break;
                    }
                    default:
                        break;
                    }
                }

                if (kill) {
                    std::cout << "Scene closed...\n";
                    break;
                }
            }
        });

        // projection server..
        auto proj_server_thread = std::thread([&]() {
            // only host if there is actually a projection data callback
            if (!projection_data_callback_) {
                return;
            }

            std::cout << "Hosting projection server at localhost:5557...\n";
            zmq::socket_t socket(context_, ZMQ_REP);
            socket.bind("tcp://*:5557");

            while (true) {
                zmq::message_t request;

                //  Wait for next request from client
                socket.recv(&request);

                zmq::message_t reply(sizeof(int));
                int success = 1;
                memcpy(reply.data(), &success, sizeof(int));
                socket.send(reply);

                auto desc = ((packet_desc*)request.data())[0];
                auto buffer =
                    memory_buffer(request.size(), (char*)request.data());

                switch (desc) {
                case packet_desc::projection_data: {
                    auto packet = std::make_unique<ProjectionDataPacket>();
                    packet->deserialize(std::move(buffer));
                    projection_data_callback_(packet->detector_pixels,
                                              packet->data,
                                              packet->projection_id);
                }
                default:
                    // ignore all other packets
                    break;
                }

            }

        });

        proj_server_thread.join();
        recast_thread.join();
    }

    void make_slice(int32_t slice_id, std::array<float, 9> orientation) {
        int32_t update_slice_index = -1;
        int32_t i = 0;
        for (auto& id_and_slice : slices_) {
            if (id_and_slice.first == slice_id) {
                update_slice_index = i;
                break;
            }
            ++i;
        }

        if (update_slice_index >= 0) {
            slices_[update_slice_index] = std::make_pair(slice_id, orientation);
        } else {
            slices_.push_back(std::make_pair(slice_id, orientation));
        }

        if (!slice_data_callback_) {
            throw server_error("No callback set");
        }

        auto result = slice_data_callback_(orientation, slice_id);

        if (!result.first.empty()) {
            auto data_packet =
                SliceDataPacket(scene_id_, slice_id, result.first, true,
                                std::move(result.second));
            send(data_packet);
        }
    }

    void set_slice_callback(callback_type callback) {
        slice_data_callback_ = callback;
    }

    void set_projection_callback(projection_callback_type callback) {
        projection_data_callback_ = callback;
    }

    void listen() {
        std::cout << "TomoPackets server listening...\n";
        serve_thread_ = std::thread([this] { this->serve(); });
    }

    int32_t scene_id() { return scene_id_; }

  private:
    // server connection
    zmq::context_t context_;
    zmq::socket_t socket_;

    // subscribe connection
    std::thread serve_thread_;
    zmq::socket_t subscribe_socket_;

    int32_t scene_id_ = -1;

    callback_type slice_data_callback_;
    std::vector<std::pair<int32_t, std::array<float, 9>>> slices_;

    projection_callback_type projection_data_callback_;
};

} // namespace tomop
