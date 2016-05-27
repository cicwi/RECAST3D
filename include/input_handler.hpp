#pragma once

namespace tomovis {

class InputHandler {
  public:
    virtual bool handle_mouse_button(int button, bool down) = 0;
    virtual bool handle_scroll(double offset) = 0;
    virtual bool handle_key(int key, bool down, int mods) = 0;
    virtual bool handle_char(unsigned int c) = 0;

    virtual int priority() const { return 10; }
};

} // namespace tomovis
