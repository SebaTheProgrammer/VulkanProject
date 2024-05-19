#include "AppBase.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>
#include "Window.h"
#include <iostream>

struct SimplePushConstantData
{
	glm::vec2 offset;
	glm::vec3 color;
};

AppBase::AppBase() : 
	WIDTH{ 800 }, HEIGHT{ 600 }, m_Window{ WIDTH, HEIGHT, std::string{"Vryens Sebastiaan Vulkan"} }
{
	LoadModels();
	CreatePipelineLayout();
	RecreateSwapChain();
	CreateCommandBuffers();
}


AppBase::~AppBase()
{
	vkDestroyPipelineLayout( m_EngineDevice.device(),
		m_PipelineLayout, nullptr );
}

void AppBase::Run()
{
    while ( !m_Window.ShouldClose() )
    {
        glfwPollEvents();
		DrawFrame();
    }

	vkDeviceWaitIdle( m_EngineDevice.device() );
}

void AppBase::LoadModels()
{
	std::vector<Model::Vertex> vertices =
	{
		{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
	};

	m_Model = std::make_unique<Model>( m_EngineDevice, vertices );
}

void AppBase::CreatePipelineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT;
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

void AppBase::CreatePipeline()
{
	assert( m_PipelineLayout != nullptr && "Cannot create pipeline before pipeline layout" );
	assert( m_SwapChain != nullptr && "Cannot create pipeline before swap chain" );


	PipelineConfigInfo pipelineConfig{};
	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = m_SwapChain->getRenderPass();
	pipelineConfig.pipelineLayout = m_PipelineLayout;
	m_Pipeline = std::make_unique<Pipeline>( 
		m_EngineDevice, 
		"shaders/shader.vert.spv",
		"shaders/shader.frag.spv",
		pipelineConfig );
}

void AppBase::RecreateSwapChain()
{
	auto extend = m_Window.GetExtent();
	while ( extend.width == 0 || extend.height == 0 )
	{
		extend = m_Window.GetExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle( m_EngineDevice.device() );

	if ( m_SwapChain == nullptr )
	{
		m_SwapChain = std::make_unique<SwapChain>( m_EngineDevice, extend );
	}
	else
	{
		m_SwapChain = std::make_unique<SwapChain>
			( m_EngineDevice, extend, std::move(m_SwapChain) );
		if ( m_SwapChain->imageCount() != m_CommandBuffers.size() )
		{
			FreeCommandBuffers();
			CreateCommandBuffers();
		}
	}
	
	CreatePipeline();
}

void AppBase::CreateCommandBuffers()
{
	m_CommandBuffers.resize( m_SwapChain->imageCount() );

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_EngineDevice.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>( m_CommandBuffers.size() );

	if ( vkAllocateCommandBuffers( m_EngineDevice.device(),
		&allocInfo, m_CommandBuffers.data() ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to allocate command buffers!" );
	}
}

void AppBase::FreeCommandBuffers()
{
	vkFreeCommandBuffers( m_EngineDevice.device(),
		m_EngineDevice.getCommandPool(),
		static_cast< uint32_t >( m_CommandBuffers.size() ),
		m_CommandBuffers.data() );

	m_CommandBuffers.clear();
}

void AppBase::RecordCommandBuffer( int imageIndex )
{
	static int frame{ 0 };
	frame = ( frame + 1 ) % 1000;

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if ( vkBeginCommandBuffer( m_CommandBuffers[ imageIndex ], &beginInfo )
		!= VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to begin recording command buffer!" );
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_SwapChain->getRenderPass();
	renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer( imageIndex );
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[ 0 ].color = { 0.01f, 0.01f, 0.01f, 1.0f };
	clearValues[ 1 ].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size() );
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass( m_CommandBuffers[ imageIndex ], &renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE );

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast< float >
		( m_SwapChain->getSwapChainExtent().width);
	viewport.height = static_cast< float >
		( m_SwapChain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0,0},m_SwapChain->getSwapChainExtent() };

	vkCmdSetViewport( m_CommandBuffers[ imageIndex ], 0, 1, &viewport );
	vkCmdSetScissor( m_CommandBuffers[ imageIndex ], 0, 1, &scissor );

	m_Pipeline->Bind( m_CommandBuffers[ imageIndex ] );
	m_Model->Bind( m_CommandBuffers[ imageIndex ] );

	for ( int index{ 0 }; index < 4; index++ )
	{
		SimplePushConstantData push{};
		push.offset = { -0.5f+frame*0.002f, -0.4f + index * 0.25f };
		push.color = { 0.0f, 0.0f, 0.2f + 0.2f * index };

		vkCmdPushConstants( m_CommandBuffers[ imageIndex ],
			m_PipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof( SimplePushConstantData ), &push );

		m_Model->Draw( m_CommandBuffers[ imageIndex ] );
	}

	vkCmdEndRenderPass( m_CommandBuffers[ imageIndex ] );

	if ( vkEndCommandBuffer( m_CommandBuffers[ imageIndex ] ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to record command buffer!" );
	}
}

void AppBase::DrawFrame()
{
	uint32_t imageIndex;
	auto result = m_SwapChain->acquireNextImage( &imageIndex );

	if ( result == VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUCCESS )
	{
		RecreateSwapChain();
		return;
	}

	RecordCommandBuffer( imageIndex );

	result = m_SwapChain->submitCommandBuffers(
		&m_CommandBuffers[ imageIndex ], &imageIndex );

	if ( result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR || m_Window.WasWindowResized()) 
	{
		m_Window.ResetWindowResizedFlag();
		RecreateSwapChain();
		return;
	}
	if ( result != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to submit command buffer!" );
	}
}
