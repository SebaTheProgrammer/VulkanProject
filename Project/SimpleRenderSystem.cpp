#include "SimpleRenderSystem.h"

#include "AppBase.h"

#include <stdexcept>
#include <array>
#include "Window.h"
#include <iostream>
#include "GameObject.h"
#include <glm/gtc/constants.hpp>
#include "Renderer.h"

struct SimplePushConstantData
{
	glm::mat2 transform{ 1.f };
	glm::vec2 offset;
	alignas( 16 ) glm::vec3 color;
};

SimpleRenderSystem::SimpleRenderSystem( EngineDevice& device,
	VkRenderPass renderPass ) 
:m_EngineDevice{ device }
{
	CreatePipelineLayout();
	CreatePipeline( renderPass );
}


SimpleRenderSystem::~SimpleRenderSystem()
{
	vkDestroyPipelineLayout( m_EngineDevice.device(),
		m_PipelineLayout, nullptr );
}


void SimpleRenderSystem::CreatePipelineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof( SimplePushConstantData );

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if ( vkCreatePipelineLayout( m_EngineDevice.device(),
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
	VkCommandBuffer commandBuffer,
	std::vector<GameObject>& gameObjects )
{
	m_Pipeline->Bind( commandBuffer );

	for ( auto& obj : gameObjects )
	{
		obj.transform2d.rotation =
			obj.transform2d.rotation + 0.01f * glm::two_pi<float>();

		SimplePushConstantData push{};
		push.offset = obj.transform2d.translation;
		push.transform = obj.transform2d.mat2();
		push.color = obj.color;

		vkCmdPushConstants( commandBuffer, m_PipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof( SimplePushConstantData ), &push );

		obj.model->Bind( commandBuffer );
		obj.model->Draw( commandBuffer );
	}
}