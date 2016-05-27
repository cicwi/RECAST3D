#include <thread>
#include <iostream>

#include "server/server.hpp"
#include "graphics/renderer.hpp"
#include "graphics/interface/interface.hpp"
#include "graphics/interface/scene_control.hpp"
#include "graphics/scene_camera.hpp"
#include "scene.hpp"
#include "input.hpp"


int main() {
    tomovis::Renderer renderer;

    auto& input = tomovis::Input::instance(renderer.window());
    renderer.register_ticker(input);

    // start server
    tomovis::Server server;
    server.start();

    // construct interface
    tomovis::Interface interface(renderer.window());
    renderer.register_target(interface);
    input.register_handler(interface);

    // construct a scene
    tomovis::Scene scene;
    renderer.register_target(scene);
    input.register_handler(scene.object().camera());

    tomovis::SceneControl scene_control(scene);
    interface.register_window(scene_control);

    // enter main loop
    renderer.main_loop();

    return 0;
}
