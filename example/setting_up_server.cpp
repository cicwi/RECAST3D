#include <functional>
#include <numeric>

// not required, CLI args parser for testing server settings
#include "flags/flags.hpp"

// include the 'slicerecon' library headers
#include "slicerecon/slicerecon.hpp"

int main(int argc, char** argv) {
    auto opts = flags::flags{argc, argv};
    opts.info("setting_up_server",
              "example of how to use `slicerecon` to host a "
              "slice reconstruction server");

    // maybe at some point support receive details over the network on acq
    // geometry, darks, flats now we just force the user to give it here..

    int n = 128;
    auto angles = std::vector<float>(n, 0.0f);
    std::iota(angles.begin(), angles.end(), 0.0f);
    std::transform(angles.begin(), angles.end(), angles.begin(),
                   [n](auto x) { return (x * M_PI) / n; });

    auto geom = slicerecon::acquisition::geometry({n, n, n, angles, true});

    // This is defined for the reconstruction
    auto slice_size = opts.arg_as_or<int32_t>("--slice-size", n);
    auto preview_size = opts.arg_as_or<int32_t>("--preview-size", 128);
    auto group_size = opts.arg_as_or<int32_t>("--group-size", 32);
    auto filter_cores = opts.arg_as_or<int32_t>("--filter-cores", 8);
    auto plugin = opts.passed("--plugin");

    auto params = slicerecon::settings{
        slice_size, preview_size, group_size, filter_cores, 1, 1};

    auto host = opts.arg_or("--host", "*");
    auto port = opts.arg_as_or<int>("--port", 5558);

    if (opts.passed("-h") || !opts.sane()) {
        std::cout << opts.usage();
        return opts.passed("-h") ? 0 : -1;
    }

    // 1. setup reconstructor
    auto recon = std::make_unique<slicerecon::reconstructor>(params);

    // 2. listen to projection stream
    // projection callback, push to projection stream
    // all raw data
    auto proj = slicerecon::projection_server(host, port, *recon, ZMQ_PULL);
    proj.serve();

    // 3. connect with (recast3d) visualization server
    auto viz = slicerecon::visualization_server(
        "slicerecon test", "tcp://localhost:5555", "tcp://localhost:5556");
    viz.set_slice_callback(
        [&](auto x, auto idx) { return recon->reconstruct_slice(x); });

    recon->add_listener(&viz);

    auto plugin_one =
        slicerecon::plugin("tcp://*:5650", "tcp://localhost:5651");
    plugin_one.set_slice_callback(
        [](auto slice, auto index) -> slicerecon::slice_data {
            auto& shape = std::get<0>(slice);
            auto& data = std::get<1>(slice);

            for (auto& x : data) {
                if (x <= 3) {
                    x = 0;
                } else {
                    x = 17;
                }
            }

            return {shape, data};
        });

    auto plugin_two = slicerecon::plugin("tcp://*:5651", "tcp://localhost:5555");
    plugin_two.set_slice_callback(
        [](auto slice, auto index) -> slicerecon::slice_data { 
            auto& shape = std::get<0>(slice);
            auto& data = std::get<1>(slice);

            for (auto& x : data) {
              msdnet.fix_everything_with_some_machines_that_were_learned(x);
                x = -x;
            }

            return {shape, data};
        });

    if (plugin) {
        viz.register_plugin("tcp://localhost:5650");
        plugin_one.serve();
        plugin_two.serve();
    }




    viz.serve();

    return 0;
}
