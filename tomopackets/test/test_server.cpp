#include "tomop/tomop.hpp"

int main() {
    // initial checks
    auto p1 = tomop::MakeScenePacket("a", 3);
    auto p2 = tomop::MakeScenePacket{};
    auto buffer = p1.serialize();
    buffer.index = 0;
    p2.deserialize(std::move(buffer));

    std::cout << p2.name << "\n";
    assert(p2.dimension == 3);

    // 1. make server
    tomop::server server("test server");

    // 2. construct some fake packages
    auto data_packet =
        tomop::SliceDataPacket(server.scene_id(), 0, {4, 4},
                               {0, 255, 128, 255, 255, 128, 255, 0, 255, 0, 128,
                                255, 255, 128, 0, 255},
                               false);

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
    server.serve();

    return 0;
}
