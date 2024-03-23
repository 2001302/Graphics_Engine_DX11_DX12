#ifndef _Input_H_
#define _Input_H_

class Input
{
public:
    Input() 
    {
        *m_keys = new bool[256] {};
    };
    Input(const Input& input) 
    {
        *m_keys = input.m_keys;
    };
    ~Input() 
    {
    };

    void Initialize();

    void KeyDown(unsigned int);
    void KeyUp(unsigned int);

    bool IsKeyDown(unsigned int);

private:
    bool m_keys[256];
};

#endif