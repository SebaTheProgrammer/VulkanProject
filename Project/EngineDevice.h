#pragma once

#include "Window.h"

// std lib headers
#include <string>
#include <vector>
#include "EngineDevice.h"

struct SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices 
{
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool IsComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class EngineDevice 
{
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  EngineDevice(Window &window);
  ~EngineDevice();

  // Not copyable or movable
  EngineDevice(const EngineDevice &) = delete;
  void operator=(const EngineDevice &) = delete;
  EngineDevice(EngineDevice &&) = delete;
  EngineDevice &operator=(EngineDevice &&) = delete;

  VkCommandPool GetCommandPool() { return m_CommandPool; }
  VkDevice Device() { return m_Device; }
  VkSurfaceKHR Surface() { return m_Surface; }
  VkQueue GraphicsQueue() { return m_GraphicsQueue; }
  VkQueue PresentQueue() { return m_PresentQueue; }

  SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(m_PhysicalDevice); }
  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(m_PhysicalDevice); }
  VkFormat FindSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  // Buffer Helper Functions
  void CreateBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer &buffer,
      VkDeviceMemory &bufferMemory);

  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void CopyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

  void CreateImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VkMemoryPropertyFlags properties,
      VkImage &image,
      VkDeviceMemory &imageMemory);
  void CreateTextureImage( VkDeviceSize size,
      VkBufferUsageFlags usage,
      VkMemoryPropertyFlags properties,
      VkBuffer& buffer,
      VkDeviceMemory& bufferMemory );
  void CreateImage(
      uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
      VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory );

  VkPhysicalDeviceProperties properties;

 private:
  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateCommandPool();
  uint32_t findMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );
  void transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels );
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands( VkCommandBuffer commandBuffer );
  void copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height );
  void generateMipmaps( VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels );
  void CreateTextureImageView();
  VkImageView createImageView( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels );
  void CreateTextureSampler();

  // helper functions
  bool IsDeviceSuitable(VkPhysicalDevice device);
  std::vector<const char *> GetRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
  void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void HasGflwRequiredInstanceExtensions();
  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

  VkInstance m_Instance;
  VkDebugUtilsMessengerEXT m_DebugMessenger;
  VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
  Window &m_Window;
  VkCommandPool m_CommandPool;

  VkDevice m_Device;
  VkSurfaceKHR m_Surface;
  VkQueue m_GraphicsQueue;
  VkQueue m_PresentQueue;

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;

  uint32_t mipLevels;
};