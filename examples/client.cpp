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

    // 2d
    auto packet = MakeScenePacket("Test", 2);
    packet.send(socket);

    //  Get the reply.
    zmq::message_t reply;
    socket.recv(&reply);

    int scene_id = *(int*)reply.data();
    std::cout << scene_id << "\n";

    std::vector<unsigned char> grayscale_image(20 * 20);
    for (auto& pixel : grayscale_image) {
        pixel = rand() % 256;
    }

    auto upd_packet =
        SliceDataPacket(scene_id, 0, {20, 20}, std::move(grayscale_image));
    upd_packet.send(socket);

    socket.recv(&reply);

    auto threedpacket = MakeScenePacket("Test 3D", 3);
    threedpacket.send(socket);

    //  Get the reply.
    socket.recv(&reply);

    scene_id = *(int*)reply.data();
    std::cout << "3D: " << scene_id << "\n";

    for (int i = 0; i < 3; ++i) {
        auto upd_packet =
            SliceDataPacket(scene_id, i, {20, 20}, std::move(grayscale_image));
        upd_packet.send(socket);
        socket.recv(&reply);
    }

    return 0;
}
