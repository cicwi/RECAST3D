// an example client

#include <cstring>
#include <iostream>
#include <string>
#include <zmq.hpp>

#include "server/server.hpp"

using namespace tomovis;

int main() {
    //  Prepare our context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);

    std::cout << "Connecting to plot server…" << std::endl;
    socket.connect("tcp://localhost:5555");


    auto packet = MakeScenePacket("Testerrr");

    auto size = packet.size();
    zmq::message_t request(size);
    memcpy(request.data(), &packet.serialize().buffer[0], size);
    socket.send(request);

    //  Get the reply.
    zmq::message_t reply;
    socket.recv(&reply);
    std::cout << *((int*)reply.data()) << "\n";

    return 0;
}
