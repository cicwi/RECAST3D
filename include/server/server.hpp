// TODO: we want some 'server system' that supports a number of operations:
// - Adding a scene,
// - Updating images / slices.
// - Benchmark data?
//
// I guess we want to use OpenMQ for this. Do we want two-way communication or
// just 'fire'? The nice thing about 'two-way' is that this 'slice
// reconstruction' thing can also be done. At first simply only 'sending' the
// appropriate slice, but later restricting construction only to the request.

#include <cstring>
#include <map>
#include <memory>
#include <queue>
#include <thread>

#include "zmq.hpp"
#include "tomop/tomop.hpp"

#include "packet_listener.hpp"
#include "ticker.hpp"

namespace tomovis {

using namespace tomop;

class SceneList;
class SceneModuleProtocol;

class Server : public Ticker, public PacketListener {
   public:
    Server(SceneList& scenes);
    void start();
    void tick(float) override;

    void register_module(std::shared_ptr<SceneModuleProtocol> module);

    void handle(Packet& pkt) override {
        try {
            auto pkt_size = pkt.size();
            zmq::message_t message(pkt_size);
            auto membuf = pkt.serialize(pkt_size);
            memcpy(message.data(), membuf.buffer.get(), pkt_size);
            publisher_socket_.send(message);
        } catch (const std::exception& e) {
            std::cout << "Failed sending: " << e.what() << "\n";
        }
    }

   private:
    std::map<packet_desc, std::shared_ptr<SceneModuleProtocol>> modules_;

    /** The server should have access to the list of scenes, to obtain the
     * correct one, or add a new one. */
    SceneList& scenes_;
    std::thread server_thread;

    /** Filled by server thread, performed by 'OpenGL' tread */
    std::queue<std::pair<packet_desc, std::unique_ptr<Packet>>> packets_;

    zmq::context_t context_;
    zmq::socket_t publisher_socket_;
};

}  // namespace tomovis
