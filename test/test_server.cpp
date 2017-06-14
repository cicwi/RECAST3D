#include "tomop/tomop.hpp"

int main() {
    // 1. make server
    tomop::server server("test server");

    // 2. construct some fake packages
    auto data_packet = tomop::SliceDataPacket(
        server.scene_id(), 0, {4, 4}, {0, 255, 128, 255, 255, 128, 255, 0, 255,
                                       0, 128, 255, 255, 128, 0, 255});

    // 3. send packages
    server.send(data_packet);

    // 4. add some callback
    auto callback = [](auto orientation) {
        return std::make_pair(std::vector<int32_t>{4, 4},
                              std::vector<uint32_t>{0, 255, 128, 255, 255, 128,
                                                   255, 0, 255, 0, 128, 255,
                                                   255, 128, 0, 255});
    };
    server.set_slice_callback(callback);

    // 5. test callback
    server.listen();

    return 0;
}
