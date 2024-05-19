#pragma once
#include <memory>
#include <vector>
#include "Window.h"
#include "EngineDevice.h"
#include "SwapChain.h"
#include "Model.h"
#include "GameObject.h"
#include "Renderer.h"

class AppBase
{
public:
    AppBase();
    ~AppBase();

    AppBase( const AppBase& ) = delete;
    AppBase( AppBase&& ) = delete;
    AppBase& operator=( const AppBase& ) = delete;
    AppBase& operator=( AppBase&& ) = delete;

    void Run();

private:
    void LoadGameObjects();

    const int WIDTH;
    const int HEIGHT;

    Window m_Window;
    EngineDevice m_EngineDevice{ m_Window };
    Renderer m_Renderer{ m_Window, m_EngineDevice };

    std::vector<GameObject> m_GameObjects;
};