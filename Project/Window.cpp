#include "Window.h"
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

Window::Window( const int w, const int h, std::string& name ) : m_Width{ w }, m_Height{ h }, m_WindowName{ name }
{
	InitWindow();
}

Window::~Window()
{
	glfwDestroyWindow( m_Window );
	glfwTerminate();
}

void Window::CreateWindowSurface( VkInstance instance, VkSurfaceKHR* surface )
{
	if ( glfwCreateWindowSurface( instance, m_Window, nullptr, surface ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Failed to create window surface!" );
	}
}

void Window::InitWindow()
{
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow( m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr );
	glfwSetWindowUserPointer( m_Window, this );
	glfwSetFramebufferSizeCallback( m_Window, FramebufferResizeCallback );
}

void Window::FramebufferResizeCallback( GLFWwindow* window, int width, int height )
{
	auto app = reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) );
	app->m_FrameBufferResized = true;
	app->m_Width = width;
	app->m_Height = height;
}
