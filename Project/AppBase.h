#pragma once
#include <memory>
#include <vector>
#include "Window.h"
#include "EngineDevice.h"
#include "SwapChain.h"
#include "Model.h"
#include "GameObject.h"
#include "Renderer.h"
#include "Descriptors.h"

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

    std::vector<GameObject>& GetGameObjects() { return m_GameObjects; }

private:
    void LoadGameObjects();

    const int WIDTH;
    const int HEIGHT;

    Window m_Window;
    EngineDevice m_EngineDevice{ m_Window };
    Renderer m_Renderer{ m_Window, m_EngineDevice };

    std::unique_ptr<DescriptorPool> m_GlobalDescriptorPool;

    std::vector<GameObject> m_GameObjects;
};