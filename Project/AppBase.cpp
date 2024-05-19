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
#include "Camera.h"
#include <chrono>
#include "Input.h"

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
    Camera camera{};
    auto viewer = GameObject::Create();
    MovementController movementController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

    while ( !m_Window.ShouldClose() )
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>( newTime - currentTime ).count();
        currentTime = newTime;
        frameTime = std::min( frameTime, 0.1f );

        movementController.MoveInPlaneXZ( m_Window.GetGLFWwindow(),
            frameTime, viewer );
        camera.SetViewYXZ( 
            viewer.transform.translation,
            viewer.transform.rotation);

        float aspect = m_Renderer.GetAspectRatio();
        camera.SetPerspectiveProjection(
			glm::radians( 45.f ), aspect, 0.1f, 100.f );

		if ( auto commandBuffer = m_Renderer.BeginFrame() ) 
		{
			m_Renderer.BeginSwapChainRenderPass( commandBuffer );
			simpleRenderSystem.RenderGameObjects
			( commandBuffer, m_GameObjects, camera );
			m_Renderer.EndSwapChainRenderPass( commandBuffer );
			m_Renderer.EndFrame();
		}
    }

	vkDeviceWaitIdle( m_EngineDevice.device() );
}

std::unique_ptr<Model> createCubeModel( EngineDevice& device, glm::vec3 offset ) {

    Model::VerticesIndices modelData{};

    modelData.vertices = {
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    };
    for ( auto& v : modelData.vertices ) {
        v.position += offset;
    }

    modelData.indices =
    { 0,  1,  2,  0,  3,  1,  4,  5,  6,
        4,  7,  5,  8,  9,  10, 8,  11, 9,
    12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 
        19, 17, 20, 21, 22, 20, 23, 21 };

    return std::make_unique<Model>( device, modelData );
}

void AppBase::LoadGameObjects()
{
	std::shared_ptr<Model> model = 
        createCubeModel( m_EngineDevice, {0.f, 0.f, 0.f} );
    auto cube = GameObject::Create();
    cube.model = model;
    cube.transform.translation = { 0.f, 0.f, 2.5f };
    cube.transform.scale = { 0.5f, 0.5f, 0.5f };

    m_GameObjects.emplace_back( std::move( cube ) );
}