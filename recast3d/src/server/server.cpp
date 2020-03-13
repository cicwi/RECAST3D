#include <iostream>
#include <thread>
#include <type_traits>

#include "tomop/tomop.hpp"
#include "zmq.hpp"

#include "modules/scene_module.hpp"
#include "scene.hpp"
#include "scene_list.hpp"
#include "server/server.hpp"

namespace tomovis {

using namespace tomop;

Server::Server(SceneList& scenes)
    : scenes_(scenes), context_(), publisher_socket_(context_, ZMQ_PUB) {
    scenes_.add_listener(this);
}

void Server::register_module(std::shared_ptr<SceneModuleProtocol> module) {
    for (auto desc : module->descriptors()) {
        modules_[desc] = module;
    }
}

void Server::start() {
    //  Prepare our context and socket
    std::cout << "Listening for incoming connections..\n";

    // todo graceful shutdown, probably by sending a 'kill' packet to self
    server_thread = std::thread([&]() {
        zmq::socket_t socket(context_, ZMQ_REP);
        socket.bind("tcp://*:5555");

        while (true) {
            zmq::message_t request;

            //  Wait for next request from client
            socket.recv(&request);
            auto desc = ((packet_desc*)request.data())[0];
            auto buffer = memory_buffer(request.size(), (char*)request.data());

            if (modules_.find(desc) == modules_.end()) {
                std::cout << "Unsupported package descriptor: "
                          << (std::underlying_type<decltype(desc)>::type)desc
                          << "\n";
                continue;
            }

            // forward the packet to the handler
            packets_.push({desc, std::move(modules_[desc]->read_packet(
                                     desc, buffer, socket, scenes_))});
        }
    });

    publisher_socket_.bind("tcp://*:5556");
}

void Server::tick(float) {
    while (!packets_.empty()) {
        auto event_packet = std::move(packets_.front());
        packets_.pop();

        modules_[event_packet.first]->process(scenes_, event_packet.first,
                                              std::move(event_packet.second));
    }
}

} // namespace tomovis
