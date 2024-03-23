#include <windows.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

#include "System.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
    System* system;
    bool result;


    // system ��ü�� �����Ѵ�.
    system = new System();
    if (!system)
    {
        return 0;
    }

    // system ��ü�� �ʱ�ȭ�ϰ� run�� ȣ���Ѵ�.
    result = system->Initialize();
    if (result)
    {
        system->Run();
    }

    // system��ü�� �����ϰ� �޸𸮸� ��ȯ�Ѵ�.
    system->Shutdown();
    delete system;
    system = 0;

    return 0;
}
