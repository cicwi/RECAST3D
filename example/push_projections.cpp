// example applications that pushes projections to a
// server that is listening

#include <cmath>
#include <numeric>

#include "flags/flags.hpp"
#include "slicerecon/slicerecon.hpp"
#include "tomop/tomop.hpp"
#include "tomos/tomos.hpp"

int main(int argc, char** argv) {
    auto opts = flags::flags{argc, argv};
    opts.info("push_projections",
              "example for pushing projections to slicerecon");

    auto count = opts.arg_as_or<int>("--count", 128);

    if (opts.passed("-h") || !opts.sane()) {
        std::cout << opts.usage();
        return opts.passed("-h") ? 0 : -1;
    }

    auto host = opts.arg_or("--host", "localhost");
    auto port = opts.arg_as_or<int>("--port", 5558);

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Pushing from: " << host << ":" << port
                          << slicerecon::util::end_log;

    auto pub = tomop::publisher(host, port, ZMQ_PUSH);

    // 1) Send geometry info
    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Send geometry info" << slicerecon::util::end_log;

    // TODO: send proper geometry info
    auto geometry_info = tomop::GeometrySpecificationPacket(0, false, count);
    pub.send(geometry_info);

    using T = float;
    int size = count;
    auto v = tomo::volume<3_D, T>(size);
    auto g = tomo::geometry::parallel<3_D, T>(v, size);
    auto f = tomo::modified_shepp_logan_phantom<T>(v);
    auto k = tomo::dim::closest<3_D, T>(v);
    auto p = tomo::forward_projection<3_D, T>(f, g, k);

    for (auto& x : p) {
        x = std::exp(-x / (1.5 * size));
    }

    tomo::ascii_plot(p.get_projection(64));

    // 2) Send some projections
    // TODO: create (low-res) sinogram with Tomos
    auto shape = std::array<int, 2>{size, size};

    auto data = std::vector<float>(size * size, 0.0f);
    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Sending darks" << slicerecon::util::end_log;

    pub.send(tomop::ProjectionPacket(0, 0, shape, data));

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Sending flats" << slicerecon::util::end_log;

    std::fill(data.begin(), data.end(), 1.0f);
    pub.send(tomop::ProjectionPacket(1, 0, shape, data));

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Sending projections" << slicerecon::util::end_log;

    for (int i = 0; i < count; ++i) {
        pub.send(
            tomop::ProjectionPacket(2, i, shape, p.get_projection(i).data()));
    }

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Finished" << slicerecon::util::end_log;

    return 0;
}
