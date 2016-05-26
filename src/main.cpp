#include <thread>
#include <iostream>

#include "server/server.hpp"
#include "graphics/renderer.hpp"
#include "graphics/interface/interface.hpp"
#include "graphics/interface/scene_control.hpp"
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

    tomovis::SceneControl scene_control(scene);
    interface.register_window(scene_control);

    // enter main loop
    renderer.main_loop();

    return 0;
}
