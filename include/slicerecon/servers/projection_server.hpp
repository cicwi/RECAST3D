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

                    memcpy(&type, buffer + index, sizeof(int32_t));
                    index += sizeof(int32_t);

                    memcpy(&idx, buffer + index, sizeof(int32_t));
                    index += sizeof(int32_t);

                    memcpy(&shape, buffer + index,
                           sizeof(std::array<int32_t, 2>));
                    index += sizeof(std::array<int32_t, 2>);

                    // the first 4 bytes of the buffer are the size of the data,
                    // so we skip ahead (its equal to reduce(shape))
                    pool_.push_projection((proj_kind)type, idx, shape,
                                          buffer + index + sizeof(int));
                    ack();
                    break;
                }
                case tomop::packet_desc::parallel_beam_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ParallelBeamGeometryPacket>();
                    packet->deserialize(std::move(mbuffer));

                    auto geom =
                        slicerecon::acquisition::geometry({packet->rows,
                                                           packet->cols,
                                                           packet->proj_count,
                                                           packet->angles,
                                                           true,
                                                           false,
                                                           0.0f,
                                                           0.0f,
                                                           {0.0f, 0.0f}});

                    pool_.initialize(geom);
                    break;
                }
                case tomop::packet_desc::parallel_vec_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ParallelVecGeometryPacket>();
                    packet->deserialize(std::move(mbuffer));

                    auto geom =
                        slicerecon::acquisition::geometry({packet->rows,
                                                           packet->cols,
                                                           packet->proj_count,
                                                           packet->vectors,
                                                           true,
                                                           true,
                                                           0.0f,
                                                           0.0f,
                                                           {0.0f, 0.0f}});

                    pool_.initialize(geom);
                    break;
                }
                case tomop::packet_desc::cone_beam_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ConeBeamGeometryPacket>();
                    packet->deserialize(std::move(mbuffer));

                    auto geom = slicerecon::acquisition::geometry(
                        {packet->rows, packet->cols, packet->proj_count,
                         packet->angles, false, false, packet->source_origin,
                         packet->origin_det, packet->detector_size});

                    pool_.initialize(geom);
                    break;
                }
                case tomop::packet_desc::cone_vec_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ConeVecGeometryPacket>();
                    packet->deserialize(std::move(mbuffer));

                    auto geom =
                        slicerecon::acquisition::geometry({packet->rows,
                                                           packet->cols,
                                                           packet->proj_count,
                                                           packet->vectors,
                                                           true,
                                                           true,
                                                           0.0f,
                                                           0.0f,
                                                           {0.0f, 0.0f}});

                    pool_.initialize(geom);
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
