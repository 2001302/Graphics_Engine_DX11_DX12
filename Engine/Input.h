#ifndef _INPUT
#define _INPUT
#include "dataBlock.h"
#include "env.h"

namespace common {
class Input {
  public:
    Input() : mouse_x(0), mouse_y(0){};
    void SetMouse(int mouse_x, int mouse_y) {
        this->mouse_x = mouse_x;
        this->mouse_y = mouse_y;
    }
    int MouseX() { return mouse_x; }
    int MouseY() { return mouse_y; }
  private:
    int mouse_x, mouse_y;
};
} // namespace engine
#endif
