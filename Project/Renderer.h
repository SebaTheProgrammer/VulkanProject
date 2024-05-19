#pragma once
#include <memory>
#include <vector>
#include "Window.h"
#include "EngineDevice.h"
#include "SwapChain.h"
#include "Model.h"
#include <cassert>

class Renderer
{
public:
    Renderer( Window& window,
    EngineDevice& engineDevice);
    ~Renderer();

    Renderer( const Renderer& ) = delete;
    Renderer( Renderer&& ) = delete;
    Renderer& operator=( const Renderer& ) = delete;
    Renderer& operator=( Renderer&& ) = delete;

    bool HasFrameStarted() const { return m_FrameStarted; };
    int GetFrameIndex() const {
        assert( m_FrameStarted && "Cannot get currentFrameIndex when frame not in progress" ); 
        return m_CurrentFrameIndex; 
    }

    VkCommandBuffer GetCurrentCommandBuffer() const 
    {
        assert( m_FrameStarted && "Cannot get command buffer when frame in progress" );
        return m_CommandBuffers[ m_CurrentFrameIndex ];
    }

    VkRenderPass GetSwapChainRenderPass() const { return m_SwapChain->getRenderPass(); }

    VkCommandBuffer BeginFrame();
    void EndFrame();
    void BeginSwapChainRenderPass( 
        VkCommandBuffer commandBuffer );
    void EndSwapChainRenderPass(
        VkCommandBuffer commandBuffer );

private:
    void CreateCommandBuffers();
    void FreeCommandBuffers();
    void RecreateSwapChain();

    Window& m_Window;
    EngineDevice& m_EngineDevice;
    std::unique_ptr < SwapChain> m_SwapChain;

    std::vector<VkCommandBuffer> m_CommandBuffers;

    uint32_t m_CurrentImageIndex;
    int m_CurrentFrameIndex = 0;
    bool m_FrameStarted = false;
};