#ifndef _Graphics_H_
#define _Graphics_H_

#include "D3dclass.h"

//GLOBALS
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics
{
public:
    Graphics() { m_D3D = 0; };
    Graphics(const Graphics& graphics) 
    {
        m_D3D = graphics.m_D3D;
    };
    ~Graphics() {};

    bool Initialize(int, int, HWND);
    void Shutdown();
    bool Frame();

private:
    bool Render();

private:
    D3DClass* m_D3D;
};

#endif