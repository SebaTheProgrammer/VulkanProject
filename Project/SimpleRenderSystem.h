#pragma once
#include <memory>
#include <vector>

#include "Pipeline.h"
#include "EngineDevice.h"
#include "SwapChain.h"
#include "GameObject.h"
#include "Camera.h"

class SimpleRenderSystem
{
public:
    SimpleRenderSystem( EngineDevice& device,
    VkRenderPass renderPass);
    ~SimpleRenderSystem();

    SimpleRenderSystem( const SimpleRenderSystem& ) = delete;
    SimpleRenderSystem( SimpleRenderSystem&& ) = delete;

    void RenderGameObjects( VkCommandBuffer commandBuffer,
    std::vector<GameObject>& gameObjects,
        const Camera& camera);

private:
    void CreatePipelineLayout();
    void CreatePipeline( VkRenderPass renderPass );

    EngineDevice& m_EngineDevice;

    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
};