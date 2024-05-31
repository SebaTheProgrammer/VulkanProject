#pragma once
#include <memory>
#include <vector>

#include "Pipeline.h"
#include "EngineDevice.h"
#include "SwapChain.h"
#include "GameObject.h"
#include "Camera.h"
#include "FrameInfo.h"

class SimpleRenderSystem
{
public:
    SimpleRenderSystem( EngineDevice& device,
    VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout );
    ~SimpleRenderSystem();

    SimpleRenderSystem( const SimpleRenderSystem& ) = delete;
    SimpleRenderSystem( SimpleRenderSystem&& ) = delete;

    void RenderGameObjects( FrameInfo& frameinfo,
    std::vector<GameObject>& gameObjects);

private:
    void CreatePipelineLayout( VkDescriptorSetLayout globalSetLayout );
    void CreatePipeline( VkRenderPass renderPass );

    EngineDevice& m_EngineDevice;

    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
};