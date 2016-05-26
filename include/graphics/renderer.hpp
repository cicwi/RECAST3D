#include <set>

#include "graphics/render_target.hpp"


struct GLFWwindow;

namespace tomovis {

class Renderer {
  public:
      Renderer(); 
      ~Renderer();

      void main_loop();
      void register_target(RenderTarget& target);

      GLFWwindow* window() const { return window_; }

  private:
    GLFWwindow* window_;

    struct RenderCompare {
        bool operator()(const RenderTarget* lhs, const RenderTarget* rhs) const {
            return lhs->z_priority() < rhs->z_priority();
        }
    };

    std::set<RenderTarget*, RenderCompare> targets_;
};

} // namespace tomovis
