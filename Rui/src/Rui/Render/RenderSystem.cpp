#include "RenderSystem.h"

#include "Rui/Core/Application.h"

namespace Rui {

	std::unique_ptr<RenderSystem::RenderData> RenderSystem::s_Data		= nullptr;
	std::unique_ptr<Device>					  RenderSystem::s_Device	= nullptr;
	std::unique_ptr<SwapChain>				  RenderSystem::s_SwapChain = nullptr;

	void RenderSystem::Init() {
		RUI_CORE_INFO("Initializing RenderSystem!");
		s_Data		= std::make_unique<RenderData>();
		s_Device	= Device::Create();
		s_SwapChain = SwapChain::Create(Application::Get().GetDisplay().GetExtent());

		CreateDescriptorSetLayout();
		CreatePipelineLayout();
		CreatePipeline();
		CreateUniformBuffers();
		CreateDescriptorSets();
		CreateCommandBuffers();
	}

	void RenderSystem::Dispose() {
	}

	void RenderSystem::DrawTriangle() {
		uint32_t imageIndex;
		auto result = s_SwapChain->AcquireNextImage(&imageIndex);

		if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
			//framebufferResized = false;
			GetSwapChain().ReCreateSwapChain();
			return;
		} else if(result != vk::Result::eSuccess) {
			RUI_CORE_ERROR("Failed to acquire swap chain image!");
		}

		auto commandBuffer = s_Data->CommandBuffers[imageIndex];

		const VkDeviceSize offsets[1] = { 0 };

			vk::CommandBufferBeginInfo beginInfo;
			commandBuffer.begin(&beginInfo);

			vk::RenderPassBeginInfo renderPassInfo;
			renderPassInfo.renderPass = s_SwapChain->GetRenderPass();
			renderPassInfo.framebuffer = s_SwapChain->GetFrameBuffer(imageIndex);

			renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
			renderPassInfo.renderArea.extent = s_SwapChain->GetSwapChainExtent();

			vk::ClearColorValue ab = std::array<float, 4>{0.3f, 0.1f, 0.35f, 1.0f};

			std::array<vk::ClearValue, 2> clearValues{};
			clearValues[0].color = ab;
			clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

			s_Data->Pipeline->Bind(commandBuffer);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s_Data->PipelineLayout, 0, 1, &s_Data->DescriptorSets[imageIndex], 0, nullptr);

			commandBuffer.bindVertexBuffers(0, 1, &s_Data->vertexBuffer, offsets);
			commandBuffer.bindIndexBuffer(s_Data->indexBuffer, 0, vk::IndexType::eUint32);
			float time = std::chrono::steady_clock::now().time_since_epoch().count() / 1000000000.0f;
			PushConstants tmp;

			float w = Application::Get().GetDisplay().GetWidth();
			float h = Application::Get().GetDisplay().GetHeight();

			tmp.iTime = time;
			tmp.iResolution = {w, h};

			commandBuffer.pushConstants(s_Data->PipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants), &tmp);

			commandBuffer.drawIndexed(static_cast<uint32_t>(s_Data->indices.size()), 1, 0, 0, 0);

			commandBuffer.endRenderPass();
			commandBuffer.end();

		result = s_SwapChain->SubmitCommandBuffers(&s_Data->CommandBuffers[imageIndex], &imageIndex);
	}

	void RenderSystem::CreateDescriptorSetLayout() {
		vk::DescriptorSetLayoutBinding uboLayoutBinding;
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		/*VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 2;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;*/

		std::array<vk::DescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if(s_Device->GetDevice().createDescriptorSetLayout(&layoutInfo, nullptr, &s_Data->DescriptorSetLayout) != vk::Result::eSuccess) {
			RUI_CORE_ERROR("Failed to create descriptor set layout!");
		}
	}

	void RenderSystem::CreateDescriptorSets() {
		std::vector<vk::DescriptorSetLayout> layouts(s_SwapChain->ImageCount(), s_Data->DescriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = s_Data->DescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(s_SwapChain->ImageCount());
		allocInfo.pSetLayouts = layouts.data();

		s_Data->DescriptorSets.resize(s_SwapChain->ImageCount());
		if(s_Device->GetDevice().allocateDescriptorSets(&allocInfo, s_Data->DescriptorSets.data()) != vk::Result::eSuccess) {
			RUI_CORE_ERROR("Failed to allocate descriptor sets!");
		}

		for(size_t i = 0; i < s_SwapChain->ImageCount(); i++) {
			vk::DescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = s_Data->UniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};

			descriptorWrites[0].dstSet = s_Data->DescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			s_Device->GetDevice().updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void RenderSystem::CreatePipelineLayout() {
		std::vector<vk::PushConstantRange> pushConstantRanges = {
			vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants))
		};

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &s_Data->DescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

		if(s_Device->GetDevice().createPipelineLayout(&pipelineLayoutInfo, nullptr, &s_Data->PipelineLayout) != vk::Result::eSuccess) {
			RUI_CORE_ERROR("Failed to create pipeline layout!");
		}
	}

	void RenderSystem::CreatePipeline() {
		auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo(s_SwapChain->Width(), s_SwapChain->Height());

		pipelineConfig->renderPass = s_SwapChain->GetRenderPass();
		pipelineConfig->pipelineLayout = s_Data->PipelineLayout;

		s_Data->Pipeline = std::make_unique<Pipeline>("res/shaders/shader.vert.spv", "res/shaders/shader_shapes.frag.spv", pipelineConfig);
	}

	void RenderSystem::CreateUniformBuffers() {
		vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

		s_Data->UniformBuffers.resize(s_SwapChain->ImageCount());
		s_Data->UniformBuffersMemory.resize(s_SwapChain->ImageCount());

		vma::AllocationCreateInfo create_info;
		create_info.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vma::AllocationInfo info;
		vma::Allocation alloc;
		vk::BufferCreateInfo bufferInfo({}, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);

		for(size_t i = 0; i < s_SwapChain->ImageCount(); i++) {
			s_Device->m_Allocator.createBuffer(&bufferInfo, &create_info, &s_Data->UniformBuffers[i], &alloc, &info);
		}

		std::array<vk::DescriptorPoolSize, 1> poolSizes;
		poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(s_SwapChain->ImageCount());

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(s_SwapChain->ImageCount());

		if(s_Device->GetDevice().createDescriptorPool(&poolInfo, nullptr, &s_Data->DescriptorPool) != vk::Result::eSuccess) {
			RUI_CORE_ERROR("Failed to create descriptor pool!");
		}
	}

	void RenderSystem::CreateCommandBuffers() {
		s_Data->CommandBuffers.resize(s_SwapChain->ImageCount());
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = s_Device->GetCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(s_Data->CommandBuffers.size());

		if(s_Device->GetDevice().allocateCommandBuffers(&allocInfo, s_Data->CommandBuffers.data()) != vk::Result::eSuccess) {
			RUI_CORE_ERROR("Failed to allocate command buffers!");
		}

		s_Data->vertices = {
			{{0.0f, 0.0f}, {1.0f, 0.0f}},
			{{1.0f, 0.0f}, {0.0f, 1.0f}},
			{{1.0f, 1.0f}, {0.0f, 0.0f}},
			{{0.0f, 1.0f}, {1.0f, 1.0f}}
		};

		s_Data->indices = {
			0, 1, 2, 2, 3, 0
		};

		vk::BufferCreateInfo buffer_info;
		buffer_info.size = sizeof(s_Data->vertices[0]) * s_Data->vertices.size();
		buffer_info.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		vma::AllocationCreateInfo create_info;
		create_info.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vma::AllocationInfo info;
		vma::Allocation alloc;

		s_Device->m_Allocator.createBuffer(&buffer_info, &create_info, &s_Data->vertexBuffer, &alloc, &info);

		void* a;

		s_Device->m_Allocator.mapMemory(alloc, &a);
		memcpy(a, s_Data->vertices.data(), sizeof(s_Data->vertices[0]) * s_Data->vertices.size());
		s_Device->m_Allocator.unmapMemory(alloc);
		//------------------------------//
		vk::BufferCreateInfo buffer_info2;
		buffer_info2.size = sizeof(s_Data->indices[0]) * s_Data->indices.size();
		buffer_info2.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		vma::AllocationCreateInfo create_info2;
		create_info2.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vma::AllocationInfo info2;
		vma::Allocation alloc2;

		s_Device->m_Allocator.createBuffer(&buffer_info2, &create_info2, &s_Data->indexBuffer, &alloc2, &info2);

		void* a2;

		s_Device->m_Allocator.mapMemory(alloc2, &a2);
		memcpy(a2, s_Data->indices.data(), sizeof(s_Data->indices[0]) * s_Data->indices.size());
		s_Device->m_Allocator.unmapMemory(alloc2);
		const VkDeviceSize offsets[1] = { 0 };

		for(size_t i = 0; i < s_Data->CommandBuffers.size(); i++) {
			auto commandBuffer = s_Data->CommandBuffers[i];
			vk::CommandBufferBeginInfo beginInfo;
			commandBuffer.begin(&beginInfo);

			vk::RenderPassBeginInfo renderPassInfo;
			renderPassInfo.renderPass = s_SwapChain->GetRenderPass();
			renderPassInfo.framebuffer = s_SwapChain->GetFrameBuffer(static_cast<int>(i));

			renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
			renderPassInfo.renderArea.extent = s_SwapChain->GetSwapChainExtent();

			vk::ClearColorValue ab = std::array<float, 4>{0.8f, 0.4f, 0.7f, 0.5f};

			std::array<vk::ClearValue, 2> clearValues{};
			clearValues[0].color = ab;
			clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

			s_Data->Pipeline->Bind(commandBuffer);
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, s_Data->PipelineLayout, 0, 1, &s_Data->DescriptorSets[i], 0, nullptr);
			PushConstants tmp= { {1280.0f, 720.0f}, 0.0f };
			RUI_CORE_ERROR("Size of PushConstants: {0}", sizeof(PushConstants));
			commandBuffer.pushConstants(RenderSystem::GetData().PipelineLayout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstants), &tmp);
			commandBuffer.bindVertexBuffers(0, 1, &s_Data->vertexBuffer, offsets);
			commandBuffer.bindIndexBuffer(s_Data->indexBuffer, 0, vk::IndexType::eUint32);


			commandBuffer.drawIndexed(static_cast<uint32_t>(s_Data->indices.size()), 1, 0, 0, 0);

			commandBuffer.endRenderPass();
			commandBuffer.end();
		}
	}

	void RenderSystem::PrepareCompute() {
		vk::ComputePipelineCreateInfo info;
		//info.flags = vk::PipelineCreateFlagBits::e
		//RenderSystem::GetDevice().GetDevice().createComputePipeline({},)
	}
}
