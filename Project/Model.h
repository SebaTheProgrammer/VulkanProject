#pragma once
#include "EngineDevice.h"
#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Model 
{
public:
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;

		static std::vector<VkVertexInputBindingDescription> 
			GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> 
			GetAttributeDescriptions();
	};

	Model( EngineDevice& device, const std::vector<Vertex>& vertices );
	~Model();

	Model( const Model& ) = delete;
	Model& operator=( const Model& ) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	void Draw( VkCommandBuffer commandBuffer );

private:
	void CreateVertexBuffer(const std::vector<Vertex> &vertices);

	EngineDevice& m_Device;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	uint32_t m_VertexCount;

};