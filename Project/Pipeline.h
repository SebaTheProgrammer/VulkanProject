#pragma once
#include <string>
#include <vector>
#include "EngineDevice.h"

struct PipelineConfigInfo
{
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline
{
public:
	Pipeline(EngineDevice& device, 
		const std::string& vertFile, 
		const std::string& fragFile, 
		const PipelineConfigInfo& pipelineInfo );

	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	void Bind(VkCommandBuffer commandBuffer);

private:
	static std::vector<char> readFile(const std::string& file);

	void createGraphicsPipeline(
		const std::string& vertFile, 
		const std::string& fragFile,
		const PipelineConfigInfo& pipelineInfo );

	void createShaderModule(
		const std::vector<char>& code, 
		VkShaderModule* shaderModule);

	EngineDevice& m_Device;
	VkPipeline m_GraphicsPipeline;
	VkShaderModule m_VertShaderModule;
	VkShaderModule m_FragShaderModule;
};