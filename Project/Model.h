#pragma once
#include "EngineDevice.h"
#include <vector>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>

class Model 
{
public:
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 uv;

		static std::vector<VkVertexInputBindingDescription> 
			GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> 
			GetAttributeDescriptions();

		bool operator==( const Vertex& other ) const
		{
			return position == other.position &&
				color == other.color &&
				normal == other.normal &&
				uv == other.uv;
		}
	};

	struct ModelData
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		void LoadModel( const std::string& filename );

		std::vector<glm::vec3> GetTriangles() const;
	};

	static std::unique_ptr<Model> CreateModelFromFile
	( EngineDevice& device,const std::string& filename );

	Model( EngineDevice& device,
		const Model::ModelData& modelData );
	~Model();

	Model( const Model& ) = delete;
	Model& operator=( const Model& ) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	void Draw( VkCommandBuffer commandBuffer );

	ModelData GetModelData() const;

private:
	void CreateVertexBuffer(const std::vector<Vertex>& vertices);
	void CreateIndexBuffer( const std::vector<uint32_t>& indices );
	
	EngineDevice& m_Device;
	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;
	uint32_t m_VertexCount;

	bool m_HasIndexBuffer = false;
	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;
	uint32_t m_IndexCount;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	ModelData m_ModelData;
};