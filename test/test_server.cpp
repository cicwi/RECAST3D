#include "tomop/tomop.hpp"

int main() {
    // 1. make server
    tomop::server server("test server");

    // 2. construct some fake packages
    auto data_packet =
        tomop::SliceDataPacket(server.scene_id(), 0, {4, 4}, true,
                               {0, 255, 128, 255, 255, 128, 255, 0, 255, 0, 128,
                                255, 255, 128, 0, 255});

    // 3. send packages
    server.send(data_packet);

    // 4. add some callback
    auto callback = [](auto orientation, int slice_id) {
        (void)orientation;
        (void)slice_id;
        return std::make_pair(std::array<int32_t, 2>{4, 4},
                              std::vector<float>{0, 255, 128, 255, 255, 128,
                                                 255, 0, 255, 0, 128, 255, 255,
                                                 128, 0, 255});
    };
    server.set_slice_callback(callback);

    // 5. test callback
    server.listen();

    tomop::multiserver multiserver("test server", {}, {});

    return 0;
}
