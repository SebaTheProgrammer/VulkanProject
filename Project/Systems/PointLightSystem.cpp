#include "PointLightSystem.h"

#include "AppBase.h"

#include <stdexcept>
#include <array>
#include "Window.h"
#include <iostream>
#include "GameObject.h"
#include <glm/gtc/constants.hpp>
#include "Renderer.h"
#include "Camera.h"

PointLightSystem::PointLightSystem( EngineDevice& device,
	VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout )
:m_EngineDevice{ device }
{
	CreatePipelineLayout( globalSetLayout );
	CreatePipeline( renderPass );
}

PointLightSystem::~PointLightSystem()
{
	vkDestroyPipelineLayout( m_EngineDevice.Device(),
		m_PipelineLayout, nullptr );
}

void PointLightSystem::CreatePipelineLayout( VkDescriptorSetLayout globalSetLayout )
{
	//VkPushConstantRange pushConstantRange{};
	//pushConstantRange.stageFlags =
	//	VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	//pushConstantRange.offset = 0;
	//pushConstantRange.size = sizeof( SimplePushConstantData );

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if ( vkCreatePipelineLayout( m_EngineDevice.Device(),
		&pipelineLayoutInfo, nullptr, &m_PipelineLayout ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to create pipeline layout!" );
	}
}

void PointLightSystem::CreatePipeline( VkRenderPass renderPass )
{
	assert( m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout" );

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo( pipelineConfig );
	pipelineConfig.attributeDescriptions.clear();
	pipelineConfig.bindingDescriptions.clear();

	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_PipelineLayout;
	m_Pipeline = std::make_unique<Pipeline>(
		m_EngineDevice,
		"shaders/pointLight.vert.spv",
		"shaders/pointLight.frag.spv",
		pipelineConfig );
}

void PointLightSystem::Render( FrameInfo& frameinfo)
{
	m_Pipeline->Bind( frameinfo.commandBuffer );

	vkCmdBindDescriptorSets( frameinfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_PipelineLayout, 0, 1,
		&frameinfo.globalDescriptorSet, 
		0, nullptr );

	vkCmdDraw( frameinfo.commandBuffer, 6, 1, 0, 0 );
}

void PointLightSystem::Update( FrameInfo& frameinfo, GlobalUbo& ubo )
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsedTime = currentTime - lastTime;
	frameinfo.deltaTime = elapsedTime.count();
	lastTime = currentTime;

	timeAccumulator += frameinfo.deltaTime;

	if ( timeAccumulator > glm::two_pi<float>() ) {
		timeAccumulator -= glm::two_pi<float>();
	}

	float initialAngle = glm::atan( -1.f, -1.f );
	float angle = initialAngle + timeAccumulator * m_Speed;

	ubo.lightPosition.x = m_Radius * glm::cos( angle );
	ubo.lightPosition.z = m_Radius * glm::sin( angle );
}
