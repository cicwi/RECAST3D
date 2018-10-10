#include <string>
#include <thread>

#include <zmq.hpp>

#include "tomop/tomop.hpp"

#include "../util/data_types.hpp"
#include "../util/log.hpp"

namespace slicerecon {

class projection_server {
  public:
    projection_server(std::string hostname, int port, reconstructor& pool,
                      int type = ZMQ_PULL)
        : context_(1), socket_(context_, type), pool_(pool), type_(type) {
        using namespace std::string_literals;
        auto address = "tcp://"s + hostname + ":"s + std::to_string(port);

        util::log << LOG_FILE << util::lvl::info << "Binding to: " << address
                  << util::end_log;
        socket_.bind(address);

        if (type == ZMQ_SUB) {
            socket_.setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);
        }
    }

    ~projection_server() {
        if (serve_thread_.joinable()) {
            serve_thread_.join();
        }

        socket_.close();
        context_.close();
    }

    void serve() {
        serve_thread_ = std::thread([&] {
            zmq::message_t update;
            while (true) {
                socket_.recv(&update);

                auto desc = ((tomop::packet_desc*)update.data())[0];
                auto buffer = (char*)update.data();

                switch (desc) {
                case tomop::packet_desc::projection: {
                    auto index = sizeof(tomop::packet_desc);

                    /* This assumes that the projection packet from TOMOP
                       does not change, but does avoid an unnecessary copy
                     */
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

                    util::log << LOG_FILE << util::lvl::info
                              << "Projection received [(" << shape[0] << " x "
                              << shape[1] << "), " << type << ", " << idx << "]"
                              << util::end_log;

                    pool_.push_projection((proj_kind)type, idx, shape,
                                          buffer + index);
                    ack();
                    break;
                }
                default:
                    util::log << LOG_FILE << util::lvl::warning
                              << "Unknown package received" << util::end_log;
                    ack();
                    break;
                }
            }
        });
    }

    void ack() {
        if (type_ == ZMQ_REP) {
            zmq::message_t reply(sizeof(int));
            int succes = 1;
            memcpy(reply.data(), &succes, sizeof(int));
            socket_.send(reply);
        }
    }

  private:
    zmq::context_t context_;
    zmq::socket_t socket_;
    reconstructor& pool_;
    int type_;

    std::thread serve_thread_;
}; // namespace slicerecon

} // namespace slicerecon
