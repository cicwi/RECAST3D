#include <functional>
#include <numeric>

// not required, CLI args parser for testing server settings
#include "flags/flags.hpp"

// include the 'slicerecon' library headers
#include "slicerecon/slicerecon.hpp"

using namespace std::string_literals;

int main(int argc, char** argv) {
    auto opts = flags::flags{argc, argv};
    opts.info(argv[0], "example of how to use `slicerecon` to host a "
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
    auto py_plugin = opts.passed("--pyplugin");
    auto recast_host = opts.arg_or("--recast-host", "localhost");
    auto use_reqrep = opts.passed("--reqrep");
    auto retrieve_phase = opts.passed("--phase");

    auto pixel_size = opts.arg_as_or<float>("--pixelsize", 1.0f);
    auto lambda = opts.arg_as_or<float>("--lambda", 1.23984193e-9);
    auto delta = opts.arg_as_or<float>("--delta", 1e-8);
    auto beta = opts.arg_as_or<float>("--beta", 1e-10);
    auto distance = opts.arg_as_or<float>("--distance", 40.0f);

    if (slice_size < 0 || preview_size < 0 || group_size < 0 ||
        filter_cores < 0) {
        std::cout << opts.usage();
        std::cout << "ERROR: Negative parameter passed\n";
        return -1;
    }

    auto paganin =
        slicerecon::paganin_settings{pixel_size, lambda, delta, beta, distance};

    auto params = slicerecon::settings{
        slice_size, preview_size, group_size,     filter_cores, 1,
        1,          false,        retrieve_phase, paganin};

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
    auto proj = slicerecon::projection_server(host, port, *recon,
                                              use_reqrep ? ZMQ_REP : ZMQ_PULL);
    proj.serve();

    // 3. connect with (recast3d) visualization server
    auto viz = slicerecon::visualization_server(
        "slicerecon test", "tcp://"s + recast_host + ":5555"s,
        "tcp://"s + recast_host + ":5556"s);
    viz.set_slice_callback(
        [&](auto x, auto idx) { return recon->reconstruct_slice(x); });

    recon->add_listener(&viz);

    auto plugin_one =
        slicerecon::plugin("tcp://*:5650", "tcp://localhost:5651");
    plugin_one.set_slice_callback(
        [](auto shape, auto data, auto index)
            -> std::pair<std::array<int32_t, 2>, std::vector<float>> {
            for (auto& x : data) {
                if (x <= 3) {
                    x = 0;
                } else {
                    x = 17;
                }
            }

            return {shape, data};
        });

    auto plugin_two =
        slicerecon::plugin("tcp://*:5651", "tcp://"s + recast_host + ":5555"s);
    plugin_two.set_slice_callback(
        [](auto shape, auto data, auto index)
            -> std::pair<std::array<int32_t, 2>, std::vector<float>> {
            for (auto& x : data) {
                (void)x;
            }

            return {shape, data};
        });

    if (plugin) {
        viz.register_plugin("tcp://localhost:5650");
        plugin_one.serve();
        plugin_two.serve();
    }

    if (py_plugin) {
        std::cout << "Registering plugin at 5652\n";
        viz.register_plugin("tcp://localhost:5652");
    }

    viz.serve();

    return 0;
}
