#include "Renderer.h"

#include <stdexcept>
#include <array>
#include "Window.h"
#include <iostream>
#include "GameObject.h"
#include <glm/gtc/constants.hpp>

Renderer::Renderer( Window& window, EngineDevice& engineDevice )
	: m_Window{ window }, m_EngineDevice{ engineDevice }
{
	RecreateSwapChain();
	CreateCommandBuffers();
}


Renderer::~Renderer()
{
	FreeCommandBuffers();
}

void Renderer::RecreateSwapChain()
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
		std::shared_ptr<SwapChain> oldSwapChain = std::move( m_SwapChain );

		m_SwapChain = std::make_unique<SwapChain>
			( m_EngineDevice, extend, oldSwapChain );

		if ( !oldSwapChain->CompareSwapFormats( *m_SwapChain.get() ) )
		{
			throw std::runtime_error( "Swap chain image or depth format has changed!" );
		}
	}
}

void Renderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize( SwapChain::MAX_FRAMES_IN_FLIGHT );

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_EngineDevice.getCommandPool();
	allocInfo.commandBufferCount = static_cast< uint32_t >( m_CommandBuffers.size() );

	if ( vkAllocateCommandBuffers( m_EngineDevice.device(),
		&allocInfo, m_CommandBuffers.data() ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to allocate command buffers!" );
	}
}

void Renderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers( m_EngineDevice.device(),
		m_EngineDevice.getCommandPool(),
		static_cast< uint32_t >( m_CommandBuffers.size() ),
		m_CommandBuffers.data() );

	m_CommandBuffers.clear();
}

VkCommandBuffer Renderer::BeginFrame()
{
	assert( !m_FrameStarted && "Cannot call BeginFrame while frame is in progress" );
	
	auto result = m_SwapChain->acquireNextImage( 
		&m_CurrentImageIndex );

	if ( result == VK_ERROR_OUT_OF_DATE_KHR || result != VK_SUCCESS )
	{
		RecreateSwapChain();
		return nullptr;
	}

	if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR ) {
		throw std::runtime_error( "failed to acquire swap chain image!" );
	}

	m_FrameStarted = true;

	auto commandBuffer = GetCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if ( vkBeginCommandBuffer( commandBuffer, &beginInfo )
		!= VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to begin recording command buffer!" );
	}

	return commandBuffer;
}

void Renderer::EndFrame()
{
	assert( m_FrameStarted && "Cannot call EndFrame while frame is not in progress" );
	
	auto commandBuffer = GetCurrentCommandBuffer();

	if ( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to record command buffer!" );
	}

	auto result = m_SwapChain->submitCommandBuffers(
		&commandBuffer, &m_CurrentImageIndex );

	if ( result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR || 
		m_Window.WasWindowResized() )
	{
		m_Window.ResetWindowResizedFlag();
		RecreateSwapChain();
	}
	if ( result != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to submit command buffer!" );
	}

	m_FrameStarted = false;
	m_CurrentFrameIndex = ( m_CurrentFrameIndex + 1 ) 
		% SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::BeginSwapChainRenderPass( VkCommandBuffer commandBuffer )
{
	assert( m_FrameStarted && 
		"Cannot begin render pass when frame not started" );
	assert( commandBuffer == GetCurrentCommandBuffer() && 
		"Cannot begin render pass for command buffer that is from a different frane" );

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_SwapChain->getRenderPass();
	renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer( m_CurrentImageIndex );
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[ 0 ].color = { 0.01f, 0.01f, 0.01f, 1.0f };
	clearValues[ 1 ].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size() );
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass( commandBuffer, &renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE );

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast< float >
		( m_SwapChain->getSwapChainExtent().width );
	viewport.height = static_cast< float >
		( m_SwapChain->getSwapChainExtent().height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0,0},m_SwapChain->getSwapChainExtent() };

	vkCmdSetViewport( commandBuffer, 0, 1, &viewport );
	vkCmdSetScissor( commandBuffer, 0, 1, &scissor );
}

void Renderer::EndSwapChainRenderPass( VkCommandBuffer commandBuffer )
{
	assert( m_FrameStarted && 
		"Can't call endSwapChainRenderPass if frame is not in progress" );
	assert(
		commandBuffer == GetCurrentCommandBuffer() &&
		"Can't end render pass on command buffer from a different frame" );
	vkCmdEndRenderPass( commandBuffer );
}


