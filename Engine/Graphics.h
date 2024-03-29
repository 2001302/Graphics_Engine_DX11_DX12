#ifndef _Graphics_H_
#define _Graphics_H_

#include "D3dclass.h"
#include "Cameraclass.h"
#include "Modelclass.h"
#include "Colorshaderclass.h"
#include "TextureShaderClass.h"
#include "Lightshaderclass.h"
#include "Lightclass.h"

//GLOBALS
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics
{
public:
    Graphics() 
    { 
        m_Direct3D = 0;
        m_Camera = 0;
        m_Model = 0;
        m_TextureShader = 0;   
        m_LightShader = 0;
        m_Light = 0;
    };
    Graphics(const Graphics& graphics) 
    {
    };
    ~Graphics() {};

    bool Initialize(int, int, HWND);
    void Shutdown();
    bool Frame();

private:
    bool Render(float);

private:
    D3DClass* m_Direct3D;
    CameraClass* m_Camera;
    ModelClass* m_Model;
    TextureShaderClass* m_TextureShader;
    LightShaderClass* m_LightShader;
    LightClass* m_Light;
};

#endif