#include <iostream>
#include <thread>

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
            auto data = (packet_desc*)request.data();
            auto desc = data[0];
            auto payload = (char*)data;
            auto data_size = request.size();
            auto buffer = memory_buffer(data_size, payload);

            switch (desc) {
                case packet_desc::make_scene: {
                    zmq::message_t reply(sizeof(int));

                    std::unique_ptr<MakeScenePacket> packet =
                        std::make_unique<MakeScenePacket>();
                    packet->deserialize(std::move(buffer));
                    std::cout << packet->name << "\n";

                    // reserve id from scenes_
                    packet->scene_id = scenes_.reserve_id();

                    memcpy(reply.data(), &packet->scene_id, sizeof(int));
                    socket.send(reply);

                    packets_.push(std::move(packet));
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
                scenes_.add_scene(packet.name, packet.scene_id, true);
            }

            default: { break; }
        }
    }
}

}  // namespace tomovis
