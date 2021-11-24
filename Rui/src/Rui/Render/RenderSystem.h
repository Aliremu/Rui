#pragma once
#include <glm/glm.hpp>   
#include <vulkan/vulkan.hpp>

#include "Pipeline.h"
#include "Rui/Core/SwapChain.h"

namespace Rui {
	class RenderSystem {
	public:
        struct UniformBufferObject {
            glm::mat4 Model;
            glm::mat4 View;
            glm::mat4 Proj;
        };

        struct RenderData {
            vk::DescriptorSetLayout DescriptorSetLayout;

            std::unique_ptr<Pipeline> ComputePipeline;
            vk::PipelineLayout ComputePipelineLayout;

            std::unique_ptr<Pipeline> Pipeline;
            vk::PipelineLayout PipelineLayout;

            std::vector<vk::CommandBuffer> CommandBuffers;

            std::vector<vk::Buffer> UniformBuffers;
            std::vector<vk::DeviceMemory> UniformBuffersMemory;

            vk::DescriptorPool DescriptorPool;
            std::vector<vk::DescriptorSet> DescriptorSets;


            uint32_t ImageIndex = 0;
            bool IsFrameStarted = false;
        };

        static void Init();
        static void Dispose();

        static void DrawTriangle();

        inline static RenderData& GetData()      { return *s_Data; }
        inline static Device&     GetDevice()    { return *s_Device; }
        inline static SwapChain&  GetSwapChain() { return *s_SwapChain; }

        static void CreateDescriptorSetLayout();
        static void CreateDescriptorSets();
        static void CreatePipelineLayout();
        static void CreatePipeline();
        static void CreateUniformBuffers();
        static void CreateCommandBuffers();

		static void PrepareCompute();
    private:
        static std::unique_ptr<RenderData> s_Data;
        static std::unique_ptr<Device>     s_Device;
        static std::unique_ptr<SwapChain>  s_SwapChain;

        
	};
}

