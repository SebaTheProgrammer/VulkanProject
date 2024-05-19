#include "AppBase.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>
#include "Window.h"
#include <iostream>
#include "GameObject.h"
#include <glm/gtc/constants.hpp>
#include "Renderer.h"
#include "SimpleRenderSystem.h"

AppBase::AppBase() : 
	WIDTH{ 800 }, HEIGHT{ 600 }, m_Window{ WIDTH, HEIGHT, std::string{"Vryens Sebastiaan Vulkan"} }
{
	LoadGameObjects();

}

AppBase::~AppBase(){}

void AppBase::Run()
{
	SimpleRenderSystem simpleRenderSystem{ 
		m_EngineDevice, m_Renderer.GetSwapChainRenderPass() };

    while ( !m_Window.ShouldClose() )
    {
        glfwPollEvents();

		if ( auto commandBuffer = m_Renderer.BeginFrame() ) 
		{
			m_Renderer.BeginSwapChainRenderPass( commandBuffer );
			simpleRenderSystem.RenderGameObjects
			( commandBuffer, m_GameObjects);
			m_Renderer.EndSwapChainRenderPass( commandBuffer );
			m_Renderer.EndFrame();
		}
    }

	vkDeviceWaitIdle( m_EngineDevice.device() );
}

void AppBase::LoadGameObjects()
{
	std::vector<Model::Vertex> vertices =
	{
		{ { 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
		{ { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
	};

	auto model = std::make_shared<Model>( m_EngineDevice, vertices );

	auto triangle1 = GameObject::Create();
	triangle1.model = model;
	triangle1.color = { 0.1f, 0.8f, 0.1f };
	triangle1.transform2d.translation = { 0.2f, 0.0f };
	triangle1.transform2d.scale = { 0.5f, 0.5f };
	triangle1.transform2d.rotation = 
		0.25f*glm::two_pi<float>();

	m_GameObjects.push_back( std::move(triangle1) );
}