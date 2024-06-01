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
#include "Systems/SimpleRenderSystem.h"
#include "Systems/PointLightSystem.h"
#include "Camera.h"
#include <chrono>
#include "Input.h"
#include "Buffer.h"
#include <numeric>
#include "SceneLoader.h"

AppBase::AppBase() :
    WIDTH{ 800 }, HEIGHT{ 600 }, m_Window{ WIDTH, HEIGHT,
    std::string{"Vryens Sebastiaan Vulkan"} } 
{
    m_GlobalDescriptorPool = DescriptorPool::Builder( m_EngineDevice )
        .setMaxSets( SwapChain::MAX_FRAMES_IN_FLIGHT )
        .addPoolSize( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT )
        .build();

	LoadGameObjects();
}

AppBase::~AppBase(){}

void AppBase::Run()
{
    auto minOffsetAllignment = std::lcm(
        m_EngineDevice.properties.limits.minUniformBufferOffsetAlignment,
        m_EngineDevice.properties.limits.nonCoherentAtomSize );

    Buffer globalUboBuffer{ m_EngineDevice, sizeof( GlobalUbo ),
            SwapChain::MAX_FRAMES_IN_FLIGHT, 
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        minOffsetAllignment };

    globalUboBuffer.map();

    auto globalSetLayout = DescriptorSetLayout::Builder( m_EngineDevice )
		.addBinding( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS )
		.build();

    std::vector<VkDescriptorSet> globalDescriptorSets(
        SwapChain::MAX_FRAMES_IN_FLIGHT );
    for ( size_t i = 0; i < globalDescriptorSets.size(); i++ ) 
    {
        auto bufferinfo = globalUboBuffer.descriptorInfo();

        DescriptorWriter(*globalSetLayout, *m_GlobalDescriptorPool)
			.writeBuffer(0, &bufferinfo )
            .build(globalDescriptorSets[i]);
	}

	SimpleRenderSystem simpleRenderSystem{ 
		m_EngineDevice, m_Renderer.GetSwapChainRenderPass(),
    globalSetLayout->getDescriptorSetLayout()};

    PointLightSystem pointLightSystem{
    m_EngineDevice, m_Renderer.GetSwapChainRenderPass(),
    globalSetLayout->getDescriptorSetLayout() };

    Camera camera{};
    auto viewer = GameObject::Create();
    viewer.m_Transform.translation = { 0.f, -5.f, 0.f };

    MovementController movementController{m_GameObjects};
    auto currentTime = std::chrono::high_resolution_clock::now();

    MovementController physicsCube{ m_GameObjects};
    physicsCube.CanMoveWithInput( false );
    physicsCube.SetBounceStrength( 5.0f );

    while ( !m_Window.ShouldClose() )
    {
        glfwPollEvents();
        m_Window.UpdateFPS();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>( newTime - currentTime ).count();
        currentTime = newTime;
        frameTime = std::min( frameTime, 0.1f );

        if ( m_GameObjects.size() > 1 ) {
            physicsCube.UpdatePhysics( m_Window.GetGLFWwindow(), frameTime, m_GameObjects[ 1 ] );
        }

        camera.SetViewYXZ( viewer.m_Transform.translation, viewer.m_Transform.rotation);
        movementController.UpdatePhysics( m_Window.GetGLFWwindow(), frameTime, viewer );

        float aspect = m_Renderer.GetAspectRatio();
        camera.SetPerspectiveProjection(glm::radians( 45.f ), aspect, 0.1f, 10000.f );

		if ( auto commandBuffer = m_Renderer.BeginFrame() ) 
		{
            int frameIndex = m_Renderer.GetFrameIndex();
            FrameInfo frameInfo{ 
                frameIndex,  frameTime, 
                commandBuffer, camera, globalDescriptorSets[frameIndex]};

            //update
            GlobalUbo ubo{};
            ubo.projection = camera.GetProjectionMatrix();
            ubo.view = camera.GetViewMatrix();
            pointLightSystem.Update( frameInfo, ubo );

            globalUboBuffer.writeToIndex( 
                &ubo, frameIndex );
            globalUboBuffer.flushIndex( frameIndex );

            //render
			m_Renderer.BeginSwapChainRenderPass( commandBuffer );
			simpleRenderSystem.RenderGameObjects( frameInfo, m_GameObjects );
			pointLightSystem.Render( frameInfo );

			m_Renderer.EndSwapChainRenderPass( commandBuffer );
			m_Renderer.EndFrame();
		}
    }

	vkDeviceWaitIdle( m_EngineDevice.Device() );
}

void AppBase::LoadGameObjects()
{
    SceneLoader sceneLoader{};
    auto gameObjects = sceneLoader.LoadGameObjects( m_EngineDevice, "Models/Scene1.json");
    m_GameObjects = std::move( gameObjects );

  /*  SceneLoader sceneLoader{};
	auto gameObjects = sceneLoader.LoadInstancedGameObjects( m_EngineDevice, "Models/Scene2.json",true, 1000 );
	m_GameObjects = std::move( gameObjects );*/

    //std::shared_ptr<Model> arena =
    //    Model::CreateModelFromFile(
    //        m_EngineDevice, "Models/Arena.obj" );
    //auto gameObject = GameObject::Create();
    //gameObject.m_Model = arena;
    //gameObject.m_Transform.translation = { 0.f, 0.0f, 0.f };
    //gameObject.m_Transform.scale = glm::vec3( 3.f );
    //m_GameObjects.emplace_back( std::move( gameObject ) );
}