#include <thread>
#include <iostream>

#include "server/server.hpp"
#include "graphics/renderer.hpp"
#include "graphics/interface/interface.hpp"
#include "scene.hpp"


int main() {
    tomovis::Renderer renderer;

    // start server
    tomovis::Server server;
    server.start();

    // construct interface
    tomovis::Interface interface(renderer.window());
    renderer.register_target(interface);

    // construct a scene
    tomovis::Scene scene;
    renderer.register_target(scene);

    // enter main loop
    renderer.main_loop();

    return 0;
}
