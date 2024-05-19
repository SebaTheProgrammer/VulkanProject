#pragma once
#include <memory>
#include <vector>
#include "Window.h"
#include "Pipeline.h"
#include "EngineDevice.h"
#include "SwapChain.h"
#include "Model.h"
#include "GameObject.h"

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
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandBuffers();
    void FreeCommandBuffers();
    void DrawFrame();
    void RecreateSwapChain();
    void RecordCommandBuffer( int imageIndex );
    void RenderGameObjects( VkCommandBuffer commandBuffer, int imageIndex );

    const int WIDTH;
    const int HEIGHT;

    Window m_Window;
    EngineDevice m_EngineDevice{ m_Window };
   std::unique_ptr < SwapChain> m_SwapChain;
   std::unique_ptr<Pipeline> m_Pipeline;
   VkPipelineLayout m_PipelineLayout;
   std::vector<VkCommandBuffer> m_CommandBuffers;
   std::vector<GameObject> m_GameObjects;
};