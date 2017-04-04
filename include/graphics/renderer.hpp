#include <set>
#include <vector>

#include "graphics/render_target.hpp"
#include "ticker.hpp"

struct GLFWwindow;

namespace tomovis {

class Renderer {
  public:
      Renderer(); 
      ~Renderer();

      void main_loop();
      void register_target(RenderTarget& target);
      void register_ticker(Ticker& ticker);

      void unregister_target(RenderTarget& target);
      void unregister_ticker(Ticker& ticker);

      GLFWwindow* window() const { return window_; }

  private:
    GLFWwindow* window_;

    struct RenderCompare {
        bool operator()(const RenderTarget* lhs, const RenderTarget* rhs) const {
            return lhs->z_priority() < rhs->z_priority();
        }
    };

    std::set<RenderTarget*, RenderCompare> targets_;
    std::vector<Ticker*> tickers_;

    double previous_time_;
};

} // namespace tomovis
