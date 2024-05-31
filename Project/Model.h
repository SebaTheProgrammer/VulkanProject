#pragma once
#include "EngineDevice.h"
#include <vector>
#include <vulkan/vulkan.h>
#include "Buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include "FrameInfo.h"

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
		std::vector<glm::vec3> triangles;

		void LoadModel( const std::string& filename );
		void LoadJSON( const std::string& filename );

		std::vector<glm::vec3> GetTriangles();

		TransformComponent m_Transform{};
		TransformComponent GetTransform() { return m_Transform; };
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
	std::unique_ptr<Buffer> m_VertexBuffer;

	uint32_t m_VertexCount;

	bool m_HasIndexBuffer = false;
	std::unique_ptr<Buffer> m_IndexBuffer;
	uint32_t m_IndexCount;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	ModelData m_ModelData;
};