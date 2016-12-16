#include <iostream>
#include <thread>

#include "packets.hpp"
#include "scene.hpp"
#include "scene_list.hpp"
#include "server/server.hpp"

#include "zmq.hpp"

namespace tomovis {

void Server::start() {
    //  Prepare our context and socket
    std::cout << "Listening on for incoming connections..\n";

    // todo graceful shutdown, probably by sending a 'kill' packet to self
    server_thread = std::thread([&]() {
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REP);
        socket.bind("tcp://*:5555");

        while (true) {
            zmq::message_t request;

            //  Wait for next request from client
            socket.recv(&request);
            auto desc = ((packet_desc*)request.data())[0];
            auto buffer = memory_buffer(request.size(), (char*)request.data());

            switch (desc) {
                case packet_desc::make_scene: {
                    zmq::message_t reply(sizeof(int));

                    auto packet = std::make_unique<MakeScenePacket>();
                    packet->deserialize(std::move(buffer));

                    // reserve id from scenes_ and return it
                    packet->scene_id = scenes_.reserve_id();
                    memcpy(reply.data(), &packet->scene_id, sizeof(int));
                    socket.send(reply);

                    packets_.push(std::move(packet));

                    break;
                }

                case packet_desc::update_image: {
                    auto packet = std::make_unique<UpdateImagePacket>();
                    packet->deserialize(std::move(buffer));
                    packets_.push(std::move(packet));

                    zmq::message_t reply(sizeof(int));
                    // reserve id from scenes_ and return it
                    int success = 1;
                    memcpy(reply.data(), &success, sizeof(int));
                    socket.send(reply);

                    break;
                }

                default: { break; }
            }
        }
    });
}

void Server::tick(float) {
    while (!packets_.empty()) {
        auto event_packet = std::move(packets_.front());
        packets_.pop();

        switch (event_packet->desc) {
            case packet_desc::make_scene: {
                MakeScenePacket& packet = *(MakeScenePacket*)event_packet.get();
                std::cout << "Making scene: " << packet.name << "\n";
                scenes_.add_scene(packet.name, packet.scene_id, true);
                break;
            }

            case packet_desc::update_image: {
                UpdateImagePacket& packet =
                    *(UpdateImagePacket*)event_packet.get();
                auto scene = scenes_.get_scene(packet.scene_id);
                if (!scene) std::cout << "Updating non-existing scene\n";
                scene->set_size(packet.image_size);
                scene->set_data(packet.data);
                break;
            }

            default: { break; }
        }
    }
}

}  // namespace tomovis
