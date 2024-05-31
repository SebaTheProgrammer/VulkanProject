#pragma once

#include "Camera.h"
#include "vulkan/vulkan.h"
#include <map>

#define MAX_LIGHTS 10

struct PointLight
{
	glm::vec4 position{};
	glm::vec4 color{};
};

struct FrameInfo
{
	int frameIndex = 0;
	float deltaTime = 0.0f;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	Camera camera{};
	VkDescriptorSet globalDescriptorSet;
};

struct GlobalUbo
{
	glm::mat4 projection{ 1.f };
	glm::mat4 view{ 1.f };
	glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, 0.2f };
	PointLight pointLights[MAX_LIGHTS];
	int pointLightCount = 0;
};
