#ifndef _INPUT
#define _INPUT

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
    void KeyPressed(int key, bool flag) { key_pressed[key] = flag; }
    bool KeyState(int key) { return key_pressed[key]; }

  private:
    int mouse_x, mouse_y;
    bool key_pressed[256] = {
        false,
    };
};
} // namespace common
#endif
