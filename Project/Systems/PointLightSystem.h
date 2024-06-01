#pragma once
#include <memory>
#include <vector>

#include "Pipeline.h"
#include "EngineDevice.h"
#include "SwapChain.h"
#include "GameObject.h"
#include "Camera.h"
#include "FrameInfo.h"

class PointLightSystem
{
public:
    PointLightSystem( EngineDevice& device,
    VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout );
    ~PointLightSystem();

    PointLightSystem( const PointLightSystem& ) = delete;
    PointLightSystem( PointLightSystem&& ) = delete;

    void Render( FrameInfo& frameinfo );
    void Update( FrameInfo& frameinfo, GlobalUbo& ubo );

private:
    void CreatePipelineLayout( VkDescriptorSetLayout globalSetLayout );
    void CreatePipeline( VkRenderPass renderPass );

    EngineDevice& m_EngineDevice;

    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;

    float timeAccumulator{};
    std::chrono::high_resolution_clock::time_point lastTime;

    float m_Radius = 200.0f;
    float m_Speed = 0.0001f;
};