#include "Pipeline.h"

#include "Rui/Core/Application.h"

namespace Rui {
    Pipeline::Pipeline(const std::string& vert_filepath, const std::string& frag_filepath, PipelineConfigInfo* config_info) {
        CreateGraphicsPipeline(vert_filepath, frag_filepath, config_info);
    }

    Pipeline::~Pipeline() {
        //vkDestroyShaderModule(RenderSystem::GetDevice().GetDevice(), m_vert_shader_module, nullptr);
        //vkDestroyShaderModule(RenderSystem::GetDevice().GetDevice(), m_frag_shader_module, nullptr);
        //vkDestroyPipeline(RenderSystem::GetDevice().GetDevice(), m_graphics_pipeline, nullptr);
    }

    std::vector<char> Pipeline::ReadFile(const std::string& filepath) {
        std::ifstream file{ filepath, std::ios::ate | std::ios::binary };
        if(!file.is_open()) {
            RUI_CORE_ERROR("Could not open file: {0}", filepath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    void Pipeline::CreateDescriptorSetLayout() {
        vk::DescriptorSetLayoutBinding uboLayoutBinding;
        uboLayoutBinding.binding            = 0;
        uboLayoutBinding.descriptorType     = vk::DescriptorType::eUniformBuffer;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.stageFlags         = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
    }

    void Pipeline::CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, PipelineConfigInfo* configInfo) {
        RUI_CORE_ASSERT(configInfo->pipelineLayout, "Cannot create graphics pipeline: no piplineLayout provided in config_info!");
        RUI_CORE_ASSERT(configInfo->renderPass, "Cannot create graphics pipeline: no renderpass provided in config_info!");

        auto vertCode = ReadFile(vertFilepath);
        auto fragCode = ReadFile(fragFilepath);

        CreateShaderModule(vertCode, &m_vert_shader_module);
        CreateShaderModule(fragCode, &m_frag_shader_module);

        vk::PipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
        shaderStages[0].module = m_vert_shader_module;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = {};
        //shaderStages[0].pNext = nullptr;
        //shaderStages[0].pSpecializationInfo = nullptr;
        
        shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
        shaderStages[1].module = m_frag_shader_module;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = {};
        //shaderStages[1].pNext = nullptr;
        //shaderStages[1].pSpecializationInfo = nullptr;

        auto bindingDescriptions     = Vertex::GetBindingDescriptions();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions      = bindingDescriptions.data();

        vk::PipelineViewportStateCreateInfo viewportInfo;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports    = &configInfo->viewport;
        viewportInfo.scissorCount  = 1;
        viewportInfo.pScissors     = &configInfo->scissor;

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
        pipelineInfo.pVertexInputState   = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo->inputAssemblyInfo;
        pipelineInfo.pViewportState      = &viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo->rasterizationInfo;
        pipelineInfo.pMultisampleState   = &configInfo->multisampleInfo;
        pipelineInfo.pColorBlendState    = &configInfo->colorBlendInfo;
        pipelineInfo.pDepthStencilState  = &configInfo->depthStencilInfo;
        pipelineInfo.pDynamicState       = nullptr;

        pipelineInfo.layout     = configInfo->pipelineLayout;
        pipelineInfo.renderPass = configInfo->renderPass;
        pipelineInfo.subpass    = configInfo->subpass;

        pipelineInfo.basePipelineIndex  = -1;
        pipelineInfo.basePipelineHandle = nullptr;

        if(RenderSystem::GetDevice().GetDevice().createGraphicsPipelines({}, 1, &pipelineInfo, nullptr, &m_graphics_pipeline) != vk::Result::eSuccess) {
            RUI_CORE_ERROR("Failed to create graphics pipeline");
        }
    }

    void Pipeline::CreateShaderModule(std::vector<char>& code, vk::ShaderModule* shaderModule) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.codeSize = code.size();
        createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

        if(RenderSystem::GetDevice().GetDevice().createShaderModule(&createInfo, nullptr, shaderModule) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create shader module");
        }
    }

    PipelineConfigInfo* Pipeline::DefaultPipelineConfigInfo(uint32_t width, uint32_t height) {
        PipelineConfigInfo* configInfo = new PipelineConfigInfo;

        configInfo->inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        configInfo->inputAssemblyInfo.primitiveRestartEnable = false;

        configInfo->viewport.x        = 0.0f;
        configInfo->viewport.y        = 0.0f;
        configInfo->viewport.width    = static_cast<float>(width);
        configInfo->viewport.height   = static_cast<float>(height);
        configInfo->viewport.minDepth = 0.0f;
        configInfo->viewport.maxDepth = 1.0f;

        configInfo->scissor.offset = vk::Offset2D( 0, 0 );
        configInfo->scissor.extent = vk::Extent2D(width, height);

        configInfo->rasterizationInfo.depthClampEnable        = false;
        configInfo->rasterizationInfo.rasterizerDiscardEnable = false;
        configInfo->rasterizationInfo.polygonMode             = vk::PolygonMode::eFill;
        configInfo->rasterizationInfo.lineWidth               = 1.0f;
        configInfo->rasterizationInfo.cullMode              = vk::CullModeFlagBits::eNone;
        configInfo->rasterizationInfo.frontFace               = vk::FrontFace::eClockwise;
        configInfo->rasterizationInfo.depthBiasEnable         = false;
        configInfo->rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
        configInfo->rasterizationInfo.depthBiasClamp          = 0.0f;  // Optional
        configInfo->rasterizationInfo.depthBiasSlopeFactor    = 0.0f;  // Optional

        configInfo->multisampleInfo.sampleShadingEnable   = false;
        configInfo->multisampleInfo.rasterizationSamples  = vk::SampleCountFlagBits::e1;
        configInfo->multisampleInfo.minSampleShading      = 1.0f;     // Optional
        configInfo->multisampleInfo.pSampleMask           = nullptr;  // Optional
        configInfo->multisampleInfo.alphaToCoverageEnable = false;    // Optional
        configInfo->multisampleInfo.alphaToOneEnable      = false;    // Optional

        configInfo->colorBlendAttachment.colorWriteMask      = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        configInfo->colorBlendAttachment.blendEnable         = true;
        configInfo->colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;   // Optional
        configInfo->colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;  // Optional
        configInfo->colorBlendAttachment.colorBlendOp        = vk::BlendOp::eAdd;       // Optional
        configInfo->colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;   // Optional
        configInfo->colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;  // Optional
        configInfo->colorBlendAttachment.alphaBlendOp        = vk::BlendOp::eAdd;       // Optional

        configInfo->colorBlendInfo.logicOpEnable     = false;
        configInfo->colorBlendInfo.logicOp           = vk::LogicOp::eCopy;  // Optional
        configInfo->colorBlendInfo.attachmentCount   = 1;
        configInfo->colorBlendInfo.pAttachments      = &configInfo->colorBlendAttachment;
        configInfo->colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        configInfo->colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        configInfo->colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        configInfo->colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        configInfo->depthStencilInfo.depthTestEnable       = true;
        configInfo->depthStencilInfo.depthWriteEnable      = true;
        configInfo->depthStencilInfo.depthCompareOp        = vk::CompareOp::eLess;
        configInfo->depthStencilInfo.depthBoundsTestEnable = false;
        configInfo->depthStencilInfo.minDepthBounds        = 0.0f;  // Optional
        configInfo->depthStencilInfo.maxDepthBounds        = 1.0f;  // Optional
        configInfo->depthStencilInfo.stencilTestEnable     = false;
        configInfo->depthStencilInfo.front                 = vk::StencilOpState();  // Optional
        configInfo->depthStencilInfo.back                  = vk::StencilOpState();  // Optional

        return configInfo;
    }

    void Pipeline::Bind(vk::CommandBuffer command_buffer) {
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline);
    }
}
