#include <string>
#include <thread>

#include <zmq.hpp>

#include "tomop/tomop.hpp"

#include "../util/data_types.hpp"
#include "../util/exceptions.hpp"
#include "../util/log.hpp"

namespace slicerecon {

class projection_server {
  public:
    projection_server(std::string hostname, int port, reconstructor& pool, int type = ZMQ_PULL)
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
                ack();

                auto desc = ((tomop::packet_desc*)update.data())[0];
                auto buffer = (char*)update.data();

                switch (desc) {
                case tomop::packet_desc::scan_settings: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet = std::make_unique<tomop::ScanSettingsPacket>();
                    packet->deserialize(std::move(mbuffer));

                    pool_.set_scan_settings(packet->darks, packet->flats,
                                            packet->already_linear);
                    break;
                }
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
                    break;
                }
                case tomop::packet_desc::geometry_specification: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::GeometrySpecificationPacket>();
                    packet->deserialize(std::move(mbuffer));

                    geom_.volume_min_point = packet->volume_min_point;
                    geom_.volume_max_point = packet->volume_max_point;

                    if (pool_.initialized()) {
                        util::log
                            << LOG_FILE << util::lvl::warning
                            << "Geometry specification received while "
                               "reconstruction is already initialized (ignored)"
                            << util::end_log;
                    }

                    break;
                }
                case tomop::packet_desc::parallel_beam_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ParallelBeamGeometryPacket>();

                    packet->deserialize(std::move(mbuffer));

                    if (packet->rows < 0 || packet->cols < 0) {
                        throw server_error(
                            "Invalid geometry specification received: negative "
                            "rows or columns");
                    }

                    if (packet->proj_count != (int)packet->angles.size()) {
                        throw server_error("Invalid geometry specification "
                                           "received: projection count is not "
                                           "equal to number of angles");
                    }

                    geom_.rows = packet->rows;
                    geom_.cols = packet->cols;
                    geom_.proj_count = packet->proj_count;
                    geom_.angles = packet->angles;
                    geom_.parallel = true;
                    geom_.vec_geometry = false;

                    pool_.initialize(geom_);

                    break;
                }
                case tomop::packet_desc::parallel_vec_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ParallelVecGeometryPacket>();
                    packet->deserialize(std::move(mbuffer));

                    if (packet->rows < 0 || packet->cols < 0) {
                        throw server_error(
                            "Invalid geometry specification received: negative "
                            "rows or columns");
                    }

                    if (packet->proj_count !=
                        (int)(packet->vectors.size() / 12)) {
                        throw server_error("Invalid geometry specification "
                                           "received: projection count is not "
                                           "equal to number of vectors");
                    }

                    geom_.rows = packet->rows;
                    geom_.cols = packet->cols;
                    geom_.proj_count = packet->proj_count;
                    geom_.angles = packet->vectors;
                    geom_.parallel = true;
                    geom_.vec_geometry = true;

                    pool_.initialize(geom_);

                    break;
                }
                case tomop::packet_desc::cone_beam_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ConeBeamGeometryPacket>();
                    packet->deserialize(std::move(mbuffer));

                    if (packet->rows < 0 || packet->cols < 0) {
                        throw server_error(
                            "Invalid geometry specification received: negative "
                            "rows or columns");
                    }

                    if (packet->proj_count != (int)packet->angles.size()) {
                        throw server_error("Invalid geometry specification "
                                           "received: projection count is not "
                                           "equal to number of angles");
                    }

                    geom_.rows = packet->rows;
                    geom_.cols = packet->cols;
                    geom_.proj_count = packet->proj_count;
                    geom_.angles = packet->angles;
                    geom_.parallel = false;
                    geom_.vec_geometry = false;
                    geom_.source_origin = packet->source_origin;
                    geom_.origin_det = packet->origin_det;
                    geom_.detector_size = packet->detector_size;

                    pool_.initialize(geom_);

                    break;
                }
                case tomop::packet_desc::cone_vec_geometry: {
                    auto mbuffer = tomop::memory_buffer(update.size(),
                                                        (char*)update.data());
                    auto packet =
                        std::make_unique<tomop::ConeVecGeometryPacket>();
                    packet->deserialize(std::move(mbuffer));

                    if (packet->rows < 0 || packet->cols < 0) {
                        throw server_error(
                            "Invalid geometry specification received: negative "
                            "rows or columns");
                    }

                    if (packet->proj_count !=
                        (int)(packet->vectors.size() / 12)) {
                        throw server_error("Invalid geometry specification "
                                           "received: projection count is not "
                                           "equal to number of vectors");
                    }

                    geom_.rows = packet->rows;
                    geom_.cols = packet->cols;
                    geom_.proj_count = packet->proj_count;
                    geom_.angles = packet->vectors;
                    geom_.parallel = false;
                    geom_.vec_geometry = true;

                    pool_.initialize(geom_);

                    break;
                }
                default:
                    util::log << LOG_FILE << util::lvl::warning
                              << "Unknown package received" << util::end_log;
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

    acquisition::geometry geom_ = {};
}; // namespace slicerecon

} // namespace slicerecon
