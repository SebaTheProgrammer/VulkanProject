#include "SimpleRenderSystem.h"

#include "AppBase.h"

#include <stdexcept>
#include <array>
#include "Window.h"
#include <iostream>
#include "GameObject.h"
#include <glm/gtc/constants.hpp>
#include "Renderer.h"
#include "Camera.h"

struct SimplePushConstantData
{
	glm::mat4 modelMatrix{ 1.f };
	glm::mat4 normalMatrix{ 1.f };
};

SimpleRenderSystem::SimpleRenderSystem( EngineDevice& device,
	VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout )
:m_EngineDevice{ device }
{
	CreatePipelineLayout( globalSetLayout );
	CreatePipeline( renderPass );
}

SimpleRenderSystem::~SimpleRenderSystem()
{
	vkDestroyPipelineLayout( m_EngineDevice.Device(),
		m_PipelineLayout, nullptr );
}

void SimpleRenderSystem::CreatePipelineLayout( VkDescriptorSetLayout globalSetLayout )
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( SimplePushConstantData );

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

void SimpleRenderSystem::CreatePipeline( VkRenderPass renderPass )
{
	assert( m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout" );

	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo( pipelineConfig );
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_PipelineLayout;
	m_Pipeline = std::make_unique<Pipeline>(
		m_EngineDevice,
		"shaders/shader.vert.spv",
		"shaders/shader.frag.spv",
		pipelineConfig );
}

void SimpleRenderSystem::RenderGameObjects( 
	FrameInfo& frameinfo,
	std::vector<GameObject>& gameObjects )
{
	m_Pipeline->Bind( frameinfo.commandBuffer );

	vkCmdBindDescriptorSets( frameinfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_PipelineLayout, 0, 1,
		&frameinfo.globalDescriptorSet, 
		0, nullptr );

	for ( auto& obj : gameObjects )
	{
		SimplePushConstantData push{};

		push.modelMatrix = obj.m_Transform.mat4();
		push.normalMatrix = obj.m_Transform.normalMatrix();

		vkCmdPushConstants( frameinfo.commandBuffer, m_PipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof( SimplePushConstantData ), &push );

		obj.m_Model->Bind( frameinfo.commandBuffer );
		obj.m_Model->Draw( frameinfo.commandBuffer );
	}
}