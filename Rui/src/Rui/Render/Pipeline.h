#pragma once

#include "Rui/Core/Device.h"

//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

namespace Rui {
    struct Vertex {
        glm::vec2 Position;
        glm::vec2 TexCoord;
        //glm::vec3 Normal;

        static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions() {
            std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
            bindingDescriptions[0] = vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex);

            return bindingDescriptions;
        }
        static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions() {
            return {
                {0, 0, vk::Format::eR32G32Sfloat, static_cast<uint32_t>(offsetof(Vertex, Position))},
                {1, 0, vk::Format::eR32G32Sfloat, static_cast<uint32_t>(offsetof(Vertex, TexCoord))}
                //{2, 0, vk::Format::eR32G32B32Sfloat, static_cast<uint32_t>(offsetof(Vertex, Normal))},
            };
        }
    };

    struct PipelineConfigInfo {
        vk::Viewport viewport;
        vk::Rect2D scissor;
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
        vk::PipelineMultisampleStateCreateInfo multisampleInfo;
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
        vk::PipelineDepthStencilStateCreateInfo depthStencilInfo;
        vk::PipelineLayout pipelineLayout = nullptr;
        vk::RenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
    public:
        Pipeline(const std::string& vert_filepath, const std::string& frag_filepath, PipelineConfigInfo* config_info);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        void Bind(vk::CommandBuffer command_buffer);
        inline const vk::Pipeline& GetPipeline() const { return m_graphics_pipeline; }
        static PipelineConfigInfo* DefaultPipelineConfigInfo(uint32_t width, uint32_t height);
    private:
        static std::vector<char> ReadFile(const std::string& filepath);

        void CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, PipelineConfigInfo* configInfo);
        void CreateDescriptorSetLayout();
        void CreateShaderModule(std::vector<char>& code, vk::ShaderModule* shader_module);

        vk::Pipeline m_graphics_pipeline;

        vk::ShaderModule m_vert_shader_module;
        vk::ShaderModule m_frag_shader_module;
    };
}