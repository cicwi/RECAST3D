// example applications that pushes projections to a
// server that is listening

#include "flags/flags.hpp"
#include "slicerecon/slicerecon.hpp"
#include "tomop/tomop.hpp"

int main(int argc, char** argv) {
    auto opts = flags::flags{argc, argv};
    opts.info("push_projections",
              "example for pushing projections to slicerecon");
    if (opts.passed("-h") || !opts.sane()) {
        std::cout << opts.usage();
        return opts.passed("-h") ? 0 : -1;
    }

    auto host = opts.arg_or("--host", "localhost");
    auto port = opts.arg_as_or<int>("--port", 5558);

    slicerecon::util::log << LOG_FILE << slicerecon::util::lvl::info
                          << "Pushing from: " << host << ":" << port
                          << slicerecon::util::end_log;

    // 1) Send geometry info
    auto geometry_info = tomop::GeometrySpecificationPacket(1,2,3);
    geometry_info.send();

    // 2) Send some projections
    for (int i = 0; i < 10; ++i) {
        auto packet = tomop::ProjectionPacket(1,2,3);
        packet.send();
    }

    return 0;
}
