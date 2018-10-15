// example applications that pushes projections to a
// server that is listening

#include <cmath>
#include <numeric>

#include "flags/flags.hpp"
#include "slicerecon/slicerecon.hpp"
#include "tomop/tomop.hpp"
#include "tomos/tomos.hpp"

using T = float;

int main(int argc, char** argv) {
    auto opts = flags::flags{argc, argv};
    opts.info("push_projections",
              "example for pushing projections to slicerecon");

    auto size = opts.arg_as_or<int>("--size", 128);
    bool parallel = opts.passed("--parallel");
    bool cone = opts.passed("--cone");

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

    auto p = std::unique_ptr<tomo::projections<3_D, T>>();

    if (parallel) {
        auto angles = std::vector<float>(size, 0.0f);
        std::iota(angles.begin(), angles.end(), 0.0f);
        std::transform(angles.begin(), angles.end(), angles.begin(),
                       [size](auto x) { return (x * M_PI) / size; });

        // 1b) Simulate experiment
        auto v = tomo::volume<3_D, T>(size);
        auto g = tomo::geometry::parallel<3_D, T>(v, size);
        auto f = tomo::modified_shepp_logan_phantom<T>(v);
        auto k = tomo::dim::closest<3_D, T>(v);
        p = std::make_unique<tomo::projections<3_D, T>>(
            tomo::forward_projection<3_D, T>(f, g, k));

        for (auto& x : *p) {
            x = std::exp(-x / (1.5 * size));
        }

        auto geometry_info =
            tomop::ParallelBeamGeometryPacket(0, size, size, size, angles);
        pub.send(geometry_info);

        // 2) Send some projections
        auto shape = std::array<int, 2>{size, size};

        tomo::ascii_plot(p->get_projection(size / 2));

        auto data = std::vector<float>(size * size, 0.0f);
        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                              << "Sending darks" << slicerecon::util::end_log;

        pub.send(tomop::ProjectionPacket(0, 0, shape, data));

        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                              << "Sending flats" << slicerecon::util::end_log;

        std::fill(data.begin(), data.end(), 1.0f);
        pub.send(tomop::ProjectionPacket(1, 0, shape, data));

        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                              << "Sending projections"
                              << slicerecon::util::end_log;

        for (int i = 0; i < size; ++i) {
            pub.send(tomop::ProjectionPacket(2, i, shape,
                                             p->get_projection(i).data()));
        }

        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                              << "Finished" << slicerecon::util::end_log;
    } else if (cone) {
        auto angles = std::vector<float>(size, 0.0f);
        std::iota(angles.begin(), angles.end(), 0.0f);
        std::transform(angles.begin(), angles.end(), angles.begin(),
                       [size](auto x) { return (x * 2.0f * M_PI) / size; });

        auto dos = 4.0f * size;
        auto dod = 4.0f * size;

        // 1b) Simulate experiment
        auto v = tomo::volume<3_D, T>({size, size, size},
                                      {0.5f * size, 0.5f * size, 0.5f * size},
                                      {size, size, size});
        auto g = tomo::geometry::cone_beam<T>(
            v, size, {1.5f * size, 1.5f * size}, {size, size}, dos, dod);
        auto f = tomo::modified_shepp_logan_phantom<T>(v);
        auto k = tomo::dim::closest<3_D, T>(v);
        p = std::make_unique<tomo::projections<3_D, T>>(
            tomo::forward_projection<3_D, T>(f, g, k));

        auto max = *std::max_element(p->data().begin(), p->data().end());

        for (auto& x : *p) {
            x = std::exp(-x / max);
        }

        auto geometry_info = tomop::ConeBeamGeometryPacket(
            0, size, size, size, dos, dod, {1.5f, 1.5f}, angles);
        pub.send(geometry_info);

        // 2) Send some projections
        tomo::ascii_plot(p->get_projection(size / 2));
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
                              << "Sending projections"
                              << slicerecon::util::end_log;

        for (int i = 0; i < size; ++i) {
            pub.send(tomop::ProjectionPacket(2, i, shape,
                                             p->get_projection(i).data()));
        }

        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                              << "Finished" << slicerecon::util::end_log;
    } else {
        slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::error
                              << "Choose either `--parallel` or `--cone`"
                              << slicerecon::util::end_log;
        return -1;
    }

    return 0;
}
