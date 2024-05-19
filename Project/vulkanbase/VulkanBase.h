#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "VulkanUtil.h"

// classes
#include "Renderer.h"
#include "Mesh.h"
#include "CommandPool.h"
#include "Vertex.h"

// libraries
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include <set>
#include <limits>
#include <algorithm>
#include <memory>

#include <glm/glm.hpp>
#include <array>
#include "OBJLoader.h"
#include "helperStructs.h"
#include <Renderer.h>
#include <Timer.h>

struct VulkanInitializeInfo 
{
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;
	GLFWwindow* window;
	CommandPool* pCommandPool;
	CommandBuffer* pCommandBuffer;
};

struct MeshInitializeInfo
{
	VulkanInitializeInfo vulkanInfo;
	VkQueue graphicsQueue;
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};

struct RendererInitializeInfo
{
	VulkanInitializeInfo vulkanInfo;
	MeshInitializeInfo meshInfo;

	VkQueue presentQueue;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;
};

const std::vector<const char*> validationLayers = 
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class VulkanBase 
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		Timer::GetInstance().Start();
		mainLoop();
		Timer::GetInstance().Stop();
		cleanup();
	}

private:

	std::unique_ptr<Renderer> m_Renderer;

	std::unique_ptr<CommandPool> m_pCommandPool;
	std::unique_ptr<CommandBuffer> m_pCommandBuffer;

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue;

	VkQueue m_PresentQueue;

	VkSemaphore m_ImageAvailableSemaphore;
	VkSemaphore m_RenderFinishedSemaphore;
	VkFence m_InFlightFence;

	OBJLoader m_ObjLoader;

private:
	void initVulkan() 
	{
		Timer::GetInstance();

		//week 06
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();

		//week 05
		PickPhysicalDevice();
		CreateLogicalDevice();

		//Rendering
		{
			//Command pool and command buffer
			m_pCommandPool = std::make_unique<CommandPool>( m_Device, VulkanUtil::FindQueueFamilies( m_PhysicalDevice, m_Surface ) );
			m_pCommandBuffer = std::move( m_pCommandPool->CreateCommandBuffer() );

			//Common vulkan initialization info
			VulkanInitializeInfo vulkanInitInfo{};
			vulkanInitInfo.device = m_Device;
			vulkanInitInfo.physicalDevice = m_PhysicalDevice;
			vulkanInitInfo.surface = m_Surface;
			vulkanInitInfo.window = window;
			vulkanInitInfo.pCommandPool = m_pCommandPool.get();
			vulkanInitInfo.pCommandBuffer = m_pCommandBuffer.get();

			//Initialize MeshInitializeInfo without vertices and indices
			MeshInitializeInfo meshInitInfo{};
			meshInitInfo.vulkanInfo = vulkanInitInfo;
			meshInitInfo.graphicsQueue = m_GraphicsQueue;

			//Need to make this so you can read it in from a file
			std::vector<glm::vec3> rectColors =
			{
				glm::vec3( 1.0f, 0.0f, 0.0f ),
				glm::vec3( 0.0f, 1.0f, 0.0f ),
				glm::vec3( 0.0f, 0.0f, 1.0f ),
				glm::vec3( 1.0f, 1.0f, 0.0f )
			};
			//std::vector<Vertex> rectVertices;
			//std::vector<uint16_t> rectIndices;
			auto [rectVertices, rectIndices] = Mesh::CreateRectangle( 1.0f, 1.0f, rectColors, true );

			//if ( m_ObjLoader.ParseOBJ( "models/monkey.obj", rectVertices, rectIndices, true ) )
			{
				//std::cout << "Successfully loaded OBJ file" << std::endl;
			}
			//else
			{
				//std::cout << "Failed to load OBJ file" << std::endl;
			}

			meshInitInfo.vertices = rectVertices;
			meshInitInfo.indices = rectIndices;
			std::unique_ptr<Mesh> rectangleMesh = std::make_unique<Mesh>( meshInitInfo );
			

			//Define the center color and an array of colors for the edge vertices
			glm::vec3 centerColor = glm::vec3( 1.0f, 1.0f, 1.0f );
			std::vector<glm::vec3> edgeColors =
			{
				glm::vec3( 1.0f, 0.0f, 0.0f ),
				glm::vec3( 0.0f, 1.0f, 0.0f ),
				glm::vec3( 0.0f, 0.0f, 1.0f ),
				glm::vec3( 1.0f, 1.0f, 0.0f ),
				glm::vec3( 1.0f, 0.0f, 1.0f ),
				glm::vec3( 0.0f, 1.0f, 1.0f )
			};
			int numSegments = edgeColors.size();

			auto [ovalVertices, ovalIndices] = Mesh::CreateOval( 1.0f, 0.5f, centerColor, edgeColors, numSegments, true );
			meshInitInfo.vertices = ovalVertices;
			meshInitInfo.indices = ovalIndices;
			std::unique_ptr<Mesh> ovalMesh = std::make_unique<Mesh>( meshInitInfo );

			//3d
			MeshInitializeInfo meshInitInfo3d{};
			const std::vector<Vertex> vertices3D = {
				{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
				{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
				{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
				{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

				{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
				{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
				{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
				{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
			};

			const std::vector<uint16_t> indices3D = {
				0, 1, 2, 2, 3, 0,
				4, 5, 6, 6, 7, 4
			};

			meshInitInfo3d.vertices = vertices3D;
			meshInitInfo3d.indices = indices3D;
			meshInitInfo3d.vulkanInfo = vulkanInitInfo;
			meshInitInfo3d.graphicsQueue = m_GraphicsQueue;

			std::unique_ptr<Mesh> mesh3d = std::make_unique<Mesh>( meshInitInfo3d );

			//Renderer initialization
			RendererInitializeInfo rendererInitInfo{};
			rendererInitInfo.vulkanInfo = vulkanInitInfo;
			rendererInitInfo.meshInfo = meshInitInfo;
			rendererInitInfo.presentQueue = m_PresentQueue;
			rendererInitInfo.imageAvailableSemaphore = m_ImageAvailableSemaphore;
			rendererInitInfo.renderFinishedSemaphore = m_RenderFinishedSemaphore;
			rendererInitInfo.inFlightFence = m_InFlightFence;

			m_Renderer = std::make_unique<Renderer>( rendererInitInfo, window );
			m_Renderer->SetupRenderer();

			//Add Meshes to renderer
			m_Renderer->AddMesh( std::move( rectangleMesh ) );
			m_Renderer->AddMesh( std::move( ovalMesh ) );
			m_Renderer->AddMesh( std::move( mesh3d ) );
		}

		//week 06
		m_Renderer->CreateSyncObjects();
	}

	void mainLoop() 
	{
		while (!glfwWindowShouldClose(window)) 
		{
			glfwPollEvents();
			//week 06
			Timer::GetInstance().Update();
			m_Renderer->DrawFrame();
		}
		vkDeviceWaitIdle(m_Device);
	}

	void cleanup() 
	{
		if(m_Renderer != nullptr) m_Renderer->Cleanup(); 

		if (m_pCommandPool != nullptr) m_pCommandPool->Destroy();
	
		if (enableValidationLayers) 
		{
			VulkanUtil::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}

		vkDestroyDevice(m_Device, nullptr);

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}
	
	void CreateSurface() 
	{
		if (glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	//Week 01: 
	GLFWwindow* window;

	//Important to initialize before creating the graphics pipeline 
	void initWindow();
	
	//Week 05 
	//Logical and physical device	
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	void CreateLogicalDevice();

	//Week 06
	//Main initialization		
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkSurfaceKHR m_Surface;

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	std::vector<const char*> GetRequiredExtensions();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	void CreateInstance();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};