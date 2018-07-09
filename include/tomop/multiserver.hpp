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

class multiserver {
  public:
    using callback_type =
        std::function<std::pair<std::array<int32_t, 2>, std::vector<float>>(
            std::array<float, 9>, int32_t)>;

    using projection_callback_type = std::function<void(
        std::array<int32_t, 2>, std::vector<float>, int32_t)>;

    // DONE: get list of hostnames, list of ports, set server count
    multiserver(std::string name, std::vector<std::string> hostnames,
                std::vector<std::string> subscribe_hostnames)
        : context_(1), server_count_(hostnames.size()),
          sockets_(),
          subscribe_sockets_(),
          scene_ids_(server_count_, -1),
          slices_(server_count_) {
        using namespace std::chrono_literals;

        // set socket timeout to 200 ms
        for (auto i = 0; i < server_count_; ++i) {
            sockets_.emplace_back(context_, ZMQ_REQ);
            subscribe_sockets_.emplace_back(context_, ZMQ_SUB);

            auto& socket = sockets_[i];
            socket.setsockopt(ZMQ_LINGER, 200);
            socket.connect(hostnames[i]);

            auto packet = MakeScenePacket(name, 3);
            packet.send(socket);

            // check if we get a reply within a second
            zmq::pollitem_t items[] = {{socket, 0, ZMQ_POLLIN, 0}};
            auto poll_result = zmq::poll(items, 1, 1000ms);

            if (poll_result <= 0) {
                throw server_error("Could not connect to server");
            } else {
                //  get the reply.
                zmq::message_t reply;
                socket.recv(&reply);
                scene_ids_[i] = *(int32_t*)reply.data();
            }
        }

        subscribe(subscribe_hostnames);
    }

    ~multiserver() {
        if (serve_thread_.joinable()) {
            serve_thread_.join();
        }

        for (auto i = 0; i < server_count_; ++i) {
            sockets_[i].close();
            subscribe_sockets_[i].close();
        }
        context_.close();
    }

    // DONE: send to a specific socket
    void send(const Packet& packet, int32_t server_id) {
        packet.send(sockets_[server_id]);
        zmq::message_t reply;
        sockets_[server_id].recv(&reply);
    }

    // DONE: subscribe to all
    void subscribe(std::vector<std::string> subscribe_hosts) {
        for (auto i = 0; i < server_count_; ++i) {
            auto subscribe_host = subscribe_hosts[i];

            if (scene_ids_[i] < 0) {
                throw server_error("Subscribe called for uninitialized server");
            }

            // set socket timeout to 200 ms
            subscribe_sockets_[i].setsockopt(ZMQ_LINGER, 200);

            //  Socket to talk to server
            subscribe_sockets_[i].connect(subscribe_host);

            std::vector<packet_desc> descriptors = {packet_desc::set_slice,
                                                    packet_desc::remove_slice,
                                                    packet_desc::kill_scene};

            for (auto descriptor : descriptors) {
                int32_t filter[] = {
                    (std::underlying_type<packet_desc>::type)descriptor,
                    scene_ids_[i]};
                subscribe_sockets_[i].setsockopt(ZMQ_SUBSCRIBE, filter,
                                                 sizeof(decltype(filter)));
            }
        }
    }

    void serve() {
        // DONE: recast thread for each
        // not only serve this, but also other..
        auto recast_threads = std::vector<std::thread>();
        for (auto i = 0; i < server_count_; ++i) {
            recast_threads.emplace_back([&, i] {
                    std::cout << "Running server: #" << i << "\n";
                auto& subscribe_socket_ = subscribe_sockets_[i];

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

                            if (packet->scene_id != scene_ids_[i]) {
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

                            make_slice(packet->slice_id, packet->orientation, i);
                            break;
                        }
                        case packet_desc::remove_slice: {
                            auto packet = std::make_unique<RemoveSlicePacket>();
                            packet->deserialize(std::move(buffer));

                            auto to_erase = std::find_if(
                                slices_[i].begin(), slices_[i].end(), [&](auto x) {
                                    return x.first == packet->slice_id;
                                });
                            slices_[i].erase(to_erase);
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
        }

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

                switch (desc) {
                case packet_desc::projection_data: {
                    auto packet = std::make_unique<ProjectionDataPacket>();
                    packet->deserialize(request);
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
        for (auto& recast_thread : recast_threads) {
            recast_thread.join();
        }
    }

    void make_slice(int32_t slice_id, std::array<float, 9> orientation, int server_idx) {
        int32_t update_slice_index = -1;
        int32_t i = 0;
        for (auto& id_and_slice : slices_[server_idx]) {
            if (id_and_slice.first == slice_id) {
                update_slice_index = i;
                break;
            }
            ++i;
        }

        if (update_slice_index >= 0) {
            slices_[server_idx][update_slice_index] = std::make_pair(slice_id, orientation);
        } else {
            slices_[server_idx].push_back(std::make_pair(slice_id, orientation));
        }

        if (!slice_data_callback_) {
            throw server_error("No callback set");
        }

        auto result = slice_data_callback_(orientation, slice_id);

        if (!result.first.empty()) {
            auto data_packet =
                SliceDataPacket(scene_ids_[i], slice_id, result.first, false,
                                std::move(result.second));
            send(data_packet, server_idx);
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

    std::vector<int32_t> scene_ids() { return scene_ids_; }

  private:
    // server connection
    zmq::context_t context_;

    int32_t server_count_ = -1;
    std::vector<zmq::socket_t> sockets_;

    // subscribe connection
    std::thread serve_thread_;
    std::vector<zmq::socket_t> subscribe_sockets_;

    std::vector<int32_t> scene_ids_;
    std::vector<std::vector<std::pair<int32_t, std::array<float, 9>>>> slices_;

    callback_type slice_data_callback_;
    projection_callback_type projection_data_callback_;
};

} // namespace tomop
