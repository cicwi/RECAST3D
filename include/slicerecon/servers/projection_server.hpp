#include <string>

#include <zmq.hpp>

#include "tomop/tomop.hpp"

#include "../util/data_types.hpp"
#include "../util/log.hpp"

namespace slicerecon {

class projection_server {
  public:
    projection_server(std::string hostname, int port, reconstructor& pool,
                      int type = ZMQ_PULL)
        : context_(1), socket_(context_, type), pool_(pool) {
        using namespace std::string_literals;
        auto address = "tcp://"s + hostname + ":"s + std::to_string(port);

        util::log << LOG_FILE << util::lvl::info << "Connecting to: " << address
                  << util::end_log;

        socket_.connect(address);
        if (type == ZMQ_SUB) {
            socket_.setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
        }
    }

    void serve() {
        auto kill = false;
        while (true) {
            zmq::message_t update;
            if (!socket_.recv(&update)) {
                kill = true;
            } else {
                auto desc = ((tomop::packet_desc*)update.data())[0];
                auto buffer = (char*)update.data();

                switch (desc) {
                case tomop::packet_desc::projection: {
                    auto index = sizeof(tomop::packet_desc);

                    /* This assumes that the projection packet from TOMOP does
                       not change, but does avoid an unnecessary copy */
                    int32_t type = 0;
                    int32_t idx = 0;
                    std::array<int32_t, 2> shape = {};

                    auto read = [&](auto& tgt) {
                        memcpy(&tgt, buffer + index, sizeof(decltype(tgt)));
                        index += sizeof(decltype(tgt));
                    };

                    read(type);
                    read(idx);
                    read(shape);

                    pool_.push_projection((proj_kind)type, idx, shape, buffer + index);
                    break;
                }
                default:
                    break;
                }

                if (kill) {
                    break;
                }
            }
        }
    }

  private:
    zmq::context_t context_;
    zmq::socket_t socket_;
    reconstructor& pool_;
};

} // namespace slicerecon
