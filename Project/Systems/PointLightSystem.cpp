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

struct PointLightPushConstants
{
	glm::vec4 position;
	glm::vec4 color;
	float radius;
};

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
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( PointLightPushConstants );

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

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

void PointLightSystem::Render( FrameInfo& frameinfo, std::vector<GameObject>& gameobjects )
{
	m_Pipeline->Bind( frameinfo.commandBuffer );

	vkCmdBindDescriptorSets( frameinfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_PipelineLayout, 0, 1,
		&frameinfo.globalDescriptorSet, 
		0, nullptr );

	for ( auto& gameObject : gameobjects )
	{
		if ( gameObject.m_PointLight == nullptr ) continue;

		PointLightPushConstants pushConstants{};
		pushConstants.position = glm::vec4( gameObject.m_Transform.translation, 1.0f );
		pushConstants.color = glm::vec4( gameObject.m_Color, gameObject.m_PointLight->intensity );
		pushConstants.radius = gameObject.m_Transform.scale.x;

		vkCmdPushConstants( frameinfo.commandBuffer,
			m_PipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof( PointLightPushConstants ), &pushConstants );

		vkCmdDraw( frameinfo.commandBuffer, 6, 1, 0, 0 );
	}
}

void PointLightSystem::Update( FrameInfo& frameinfo, GlobalUbo& ubo, std::vector<GameObject>& gameobjects )
{
	int lightIndex = 0;
	for ( auto& gameObject : gameobjects )
	{
		if( gameObject.m_PointLight==nullptr ) continue;

		ubo.pointLights[lightIndex].position = glm::vec4( gameObject.m_Transform.translation, 1.0f );
		ubo.pointLights[lightIndex].color = glm::vec4( gameObject.m_Color, gameObject.m_PointLight->intensity );
		
		lightIndex += 1;
	}

	ubo.pointLightCount = lightIndex;
}
