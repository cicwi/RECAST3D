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

    int n = 10;
    auto angles = std::vector<float>(n, 0.0f);
    std::iota(angles.begin(), angles.end(), 0.0f);
    std::transform(angles.begin(), angles.end(), angles.begin(),
                   [](auto x) { return x * 2 * M_PI; });

    auto geom = slicerecon::acquisition::geometry({n, n, n, angles, true});

    // This is defined for the reconstruction
    auto slice_size = opts.arg_as_or<int32_t>("--slice-size", 512);
    auto preview_size = opts.arg_as_or<int32_t>("--preview-size", 128);
    auto group_size = opts.arg_as_or<int32_t>("--group-size", 10);
    auto filter_cores = opts.arg_as_or<int32_t>("--filter-cores", 8);

    auto params = slicerecon::settings{
        slice_size, preview_size, group_size, filter_cores, 1, 1};

    auto host = opts.arg_or("--host", "*");
    auto port = opts.arg_as_or<int>("--port", 5558);

    if (opts.passed("-h") || !opts.sane()) {
        std::cout << opts.usage();
        return opts.passed("-h") ? 0 : -1;
    }

    // 1. setup reconstructor
    std::unique_ptr<slicerecon::reconstructor> recon = nullptr;
    // ... after receiving parameters
    recon = std::make_unique<slicerecon::reconstructor>(geom, params);

    // 2. listen to projection stream
    // projection callback, push to projection stream
    // all raw data
    auto proj = slicerecon::projection_server(host, port, *recon, ZMQ_PULL);
    proj.serve();

    // 3. connect with (recast3d) visualization server
    auto viz = slicerecon::visualization_server("slicerecon test");
    viz.set_slice_callback(
        [&](auto x, auto idx) { return recon->reconstruct_slice(x); });
    viz.serve();

    return 0;
}
