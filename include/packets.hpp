namespace tomovis {

enum class packet_desc : int {
    // making a scene is done by supplying a name and some
    // information on the scene, e.g.
    // 2d or 3d, or dimensions
    // the scene_id will be returned
    make_scene,
    update_image
};

class Packet {
   public:
    Packet(packet_desc desc_) : desc(desc_) {}
    packet_desc desc;

    virtual memory_buffer serialize() = 0;
    virtual void deserialize(memory_buffer buffer) = 0;
};

class MakeScenePacket : public Packet {
   public:
    MakeScenePacket() : Packet(packet_desc::make_scene) {}
    MakeScenePacket(std::string name_)
        : Packet(packet_desc::make_scene), name(name_) {}

    std::size_t size() {
        scale total;
        fill(total);
        return total.size;
    }

    memory_buffer serialize() override {
        auto result = memory_buffer(size());
        fill(result);
        return result;
    }

    // merge this and deserialize?
    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer << this->desc;
        buffer << name;
    }

    void deserialize(memory_buffer buffer) override {
        buffer >> this->desc;
        buffer >> name;
    }

    std::string name;
    int scene_id;
};

class UpdateImagePacket : public Packet {
   public:
    UpdateImagePacket() : Packet(packet_desc::update_image) {}

    std::size_t size() {
        scale total;
        fill(total);
        return total.size;
    }

    memory_buffer serialize() override {
        auto result = memory_buffer(size());
        fill(result);
        return result;
    }

    // merge this and deserialize?
    template <typename Buffer>
    void fill(Buffer& buffer) {
        buffer << this->desc;
        buffer << name;
    }

    void deserialize(memory_buffer buffer) override {
        buffer >> this->desc;
        buffer >> name;
    }

    int scene_id;
    std::vector<float> data;
};

}  // namespace tomovis
