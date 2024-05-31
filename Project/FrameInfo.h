#pragma once

#include "Camera.h"
#include "vulkan/vulkan.h"

struct FrameInfo
{
	int frameIndex = 0;
	float deltaTime = 0.0f;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	Camera camera{};
	VkDescriptorSet globalDescriptorSet;
};
