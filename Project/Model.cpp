#include "Model.h"
#include <cassert>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>
#include "Utils.h"
#define GLM_ENABLE_EXPERIMENTAL //is it still so experimental?
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include "stb_image.h"
#include "Buffer.h"
#include "json.hpp"

using json = nlohmann::json;

namespace std 
{
	template<> struct hash<Model::Vertex>
	{
		size_t operator()( Model::Vertex const& vertex ) const
		{
			size_t seed = 0;
			hashCombine( seed, vertex.position, vertex.color, vertex.normal, vertex.uv );
			return seed;
		}
	};
}

Model::Model( EngineDevice& device,
	const Model::ModelData& modelData )
	: m_Device( device )
{
	m_ModelData = modelData;
	CreateVertexBuffer( m_ModelData.vertices );
	CreateIndexBuffer( m_ModelData.indices );
}

Model::~Model(){}

void Model::Bind( VkCommandBuffer commandBuffer )
{
	VkBuffer buffers[] = { m_VertexBuffer->getBuffer()};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers( commandBuffer, 0, 1, buffers, offsets );

	if( m_HasIndexBuffer )
	{
		vkCmdBindIndexBuffer( commandBuffer,
			m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}
}

std::unique_ptr<Model> Model::CreateModelFromFile( EngineDevice& device, const std::string& filename )
{
	ModelData modelData;

	std::string extension = std::filesystem::path( filename ).extension().string();
	if ( extension == ".obj" ) {
		modelData.LoadModel( filename );
	}
	else if ( extension == ".json" ) {
		modelData.LoadJSON( filename );
	}
	else {
		throw std::runtime_error( "Unsupported file format: " + extension );
	}
	return std::make_unique<Model>( device, modelData );
}

void Model::Draw( VkCommandBuffer commandBuffer )
{
	if ( m_HasIndexBuffer )
	{
		vkCmdDrawIndexed( commandBuffer, m_IndexCount,
			1, 0, 0, 0 );
	}
	else 
	{
		vkCmdDraw( commandBuffer, m_VertexCount, 1, 0, 0 );
	}
}

Model::ModelData Model::GetModelData() const
{
	return m_ModelData;
}

void Model::CreateVertexBuffer( const std::vector<Vertex>& vertices )
{
	m_VertexCount = static_cast< uint32_t >( vertices.size() );
	assert( m_VertexCount >= 3 && "Vertex count must be at least 3 for a triangle" );
	VkDeviceSize bufferSize = sizeof( vertices[ 0 ] ) * m_VertexCount;

	uint32_t vertexSize = sizeof( vertices[ 0 ] );

	Buffer stagingBuffer( m_Device, vertexSize, m_VertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

	stagingBuffer.map();
	stagingBuffer.writeToBuffer( (void*) vertices.data() );

	m_VertexBuffer = std::make_unique<Buffer>
		( m_Device, vertexSize, m_VertexCount,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	m_Device.CopyBuffer( stagingBuffer.getBuffer(),
		m_VertexBuffer->getBuffer(), bufferSize);
}

void Model::CreateIndexBuffer( const std::vector<uint32_t>& indices )
{
	m_IndexCount = static_cast< uint32_t >( indices.size() );
	m_HasIndexBuffer = m_IndexCount > 0;

	if( !m_HasIndexBuffer )
	{
		return;
	}

	VkDeviceSize bufferSize = sizeof( indices[ 0 ] ) * m_IndexCount;

	uint32_t indexSize = sizeof( indices[ 0 ] );

	Buffer stagingBuffer( m_Device, indexSize, m_IndexCount,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

	stagingBuffer.map();
	stagingBuffer.writeToBuffer( (void*) indices.data() );

	m_IndexBuffer = std::make_unique<Buffer>
		( m_Device, indexSize, m_IndexCount,
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
					VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

	m_Device.CopyBuffer( stagingBuffer.getBuffer(),
		m_IndexBuffer->getBuffer(), bufferSize);
}

std::vector<VkVertexInputBindingDescription> 
Model::Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> 
		bindingDescriptions( 1 );
	bindingDescriptions[ 0 ].binding = 0;
	bindingDescriptions[ 0 ].stride = sizeof( Vertex );
	bindingDescriptions[ 0 ].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> 
Model::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription>
		attributeDescriptions;

	attributeDescriptions.push_back( 
		{ 0,0,VK_FORMAT_R32G32B32_SFLOAT,
		offsetof( Vertex, position )} );
	attributeDescriptions.push_back(
		{ 1,0,VK_FORMAT_R32G32B32_SFLOAT,
		offsetof( Vertex, color ) } );
	attributeDescriptions.push_back(
		{ 2,0,VK_FORMAT_R32G32B32_SFLOAT,
		offsetof( Vertex, normal ) } );
	attributeDescriptions.push_back(
		{ 3,0,VK_FORMAT_R32G32_SFLOAT,
		offsetof( Vertex, uv ) } );

	return attributeDescriptions;
}

void Model::ModelData::LoadModel( const std::string& filename )
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &err, filename.c_str() ) )
	{
		std::cout << warn + err << std::endl;
		throw std::runtime_error( warn + err );
	}

	vertices.clear();
	indices.clear();

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for ( const auto& shape : shapes )
	{
		for ( const auto& index : shape.mesh.indices )
		{
			Vertex vertex{};

			if ( index.vertex_index >= 0 )
			{
				vertex.position = 
				{
					attrib.vertices[ 3 * index.vertex_index + 0 ],
					attrib.vertices[ 3 * index.vertex_index + 1 ],
					attrib.vertices[ 3 * index.vertex_index + 2 ]
				};

				vertex.color = 
				{ 
					attrib.colors[ 3 * index.vertex_index + 0 ],
					attrib.colors[ 3 * index.vertex_index + 1 ],
					attrib.colors[ 3 * index.vertex_index + 2 ]
				};
			}

			vertex.position.y *= -1.0;

			if ( index.normal_index >= 0 )
			{
				vertex.normal =
				{
					attrib.normals[ 3 * index.normal_index + 0 ],
					attrib.normals[ 3 * index.normal_index + 1 ],
					attrib.normals[ 3 * index.normal_index + 2 ]
				};
			}

			if ( index.texcoord_index >= 0 )
			{
				vertex.uv =
				{
					attrib.texcoords[ 2 * index.texcoord_index + 0 ],
					attrib.texcoords[ 2 * index.texcoord_index + 1 ],
				};
			}

			if ( uniqueVertices.count( vertex ) == 0 )
			{
				uniqueVertices[ vertex ]= static_cast< uint32_t >( vertices.size() );
				vertices.push_back( vertex );
			}

			indices.push_back( uniqueVertices[ vertex ] );
		}
	}

	triangles = GetTriangles();

	//// TODO: Load the image using a library like STB image
	//int texWidth, texHeight, texChannels;
	//stbi_uc* pixels = stbi_load( "texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

	//// Create Vulkan image object
	//VkImage textureImage;
	//VkDeviceMemory textureImageMemory;
	//createImage( texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory );

	//// Copy texture data to image
	//transitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
	//copyBufferToImage( pixels, texWidth, texHeight, textureImage );
	//transitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

	//// Create image view
	//VkImageView textureImageView = createImageView( textureImage, VK_FORMAT_R8G8B8A8_UNORM );

	//// Create sampler
	//VkSampler textureSampler = createSampler();

	//// Bind texture to shader (this is done in your shader code)
}

void  Model::ModelData::LoadJSON( const std::string& filename )
{
	std::ifstream file( filename );
	if ( !file.is_open() ) {
		throw std::runtime_error( "Failed to open JSON file: " + filename );
	}

	json jsonData;
	file >> jsonData;

	std::string objFilePath = jsonData[ "obj_file_path" ].get<std::string>();
	glm::vec3 location = glm::vec3( jsonData[ "location" ][ 0 ], jsonData[ "location" ][ 1 ], jsonData[ "location" ][ 2 ] );
	float scale = jsonData[ "scale" ];

	Model::ModelData::m_Transform.translation = location;
	Model::ModelData::m_Transform.scale = glm::vec3( scale );

	LoadModel( objFilePath );
}

std::vector<glm::vec3> Model::ModelData::GetTriangles()
{
	std::vector<glm::vec3> triangles;

	for ( size_t i = 0; i < indices.size(); i += 3 )
	{
		uint32_t index0 = indices[ i ];
		uint32_t index1 = indices[ i + 1 ];
		uint32_t index2 = indices[ i + 2 ];

		glm::vec3 v0 = vertices[ index0 ].position;
		glm::vec3 v1 = vertices[ index1 ].position;
		glm::vec3 v2 = vertices[ index2 ].position;

		triangles.push_back( v0 );
		triangles.push_back( v1 );
		triangles.push_back( v2 );
	}

	return triangles;
}
