#include "AppBase.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <stdexcept>
#include <array>
#include "Window.h"
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
    MovementController movementController{m_GameObjects};

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
            viewer.m_Transform.translation,
            viewer.m_Transform.rotation);

        float aspect = m_Renderer.GetAspectRatio();
        camera.SetPerspectiveProjection(
			glm::radians( 45.f ), aspect, 0.1f, 1000.f );

		if ( auto commandBuffer = m_Renderer.BeginFrame() ) 
		{
			m_Renderer.BeginSwapChainRenderPass( commandBuffer );
			simpleRenderSystem.RenderGameObjects
			( commandBuffer, m_GameObjects, camera );
			m_Renderer.EndSwapChainRenderPass( commandBuffer );
			m_Renderer.EndFrame();
		}
    }

	vkDeviceWaitIdle( m_EngineDevice.Device() );
}

void AppBase::LoadGameObjects()
{
	std::shared_ptr<Model> model = 
       Model::CreateModelFromFile(
           m_EngineDevice, "C:/Users/vryen/Desktop/Restart/VulkanProject/Project/Models/colored_cube.obj" );

    auto gameObject = GameObject::Create();
    gameObject.m_Model = model;
    gameObject.m_Transform.translation = { 0.f, 0.f, 0.f };
    gameObject.m_Transform.scale = glm::vec3( 1.f );

    m_GameObjects.emplace_back( std::move( gameObject ) );
}