#include "SwapChain.h"

#include "Application.h"

namespace Rui {

    SwapChain::SwapChain(vk::Extent2D extent)
        : windowExtent{ extent } {
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDepthResources();
        CreateFramebuffers();
        CreateSyncObjects();
    }

    SwapChain::~SwapChain() {
        for(auto imageView : swapChainImageViews) {
            vkDestroyImageView(RenderSystem::GetDevice().GetDevice(), imageView, nullptr);
        }
        swapChainImageViews.clear();

        /*if(swapChain != nullptr) {
            vkDestroySwapchainKHR(RenderSystem::GetDevice().GetDevice(), swapChain, nullptr);
            swapChain = nullptr;
        }*/

        for(int i = 0; i < depthImages.size(); i++) {
            vkDestroyImageView(RenderSystem::GetDevice().GetDevice(), depthImageViews[i], nullptr);
            vkDestroyImage(RenderSystem::GetDevice().GetDevice(), depthImages[i], nullptr);
            vkFreeMemory(RenderSystem::GetDevice().GetDevice(), depthImageMemorys[i], nullptr);
        }

        for(auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(RenderSystem::GetDevice().GetDevice(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(RenderSystem::GetDevice().GetDevice(), renderPass, nullptr);

        // cleanup synchronization objects
        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(RenderSystem::GetDevice().GetDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(RenderSystem::GetDevice().GetDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(RenderSystem::GetDevice().GetDevice(), inFlightFences[i], nullptr);
        }
    }

    vk::Result SwapChain::AcquireNextImage(uint32_t* imageIndex) {
        RenderSystem::GetDevice().GetDevice().waitForFences(
            1,
            &inFlightFences[currentFrame],
            true,
            std::numeric_limits<uint64_t>::max());

        vk::Result result = RenderSystem::GetDevice().GetDevice().acquireNextImageKHR(
            swapChain,
            std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
            nullptr,
            imageIndex);

        return result;
    }

    vk::Result SwapChain::SubmitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t* imageIndex) {
        if(imagesInFlight[*imageIndex]) {
            RenderSystem::GetDevice().GetDevice().waitForFences(1, &imagesInFlight[*imageIndex], true, UINT64_MAX);
        }
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

        vk::SubmitInfo submitInfo;
        vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        RenderSystem::GetDevice().GetDevice().resetFences(1, &inFlightFences[currentFrame]);

    	if(RenderSystem::GetDevice().GraphicsQueue().submit(1, &submitInfo, inFlightFences[currentFrame]) != vk::Result::eSuccess) {
    		RUI_CORE_ERROR("Failed to submit draw command buffer!");
    	}

        vk::PresentInfoKHR presentInfo;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        vk::SwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = imageIndex;

        auto result = RenderSystem::GetDevice().PresentQueue().presentKHR(&presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void SwapChain::CreateSwapChain() {
        SwapChainSupportDetails swapChainSupport = RenderSystem::GetDevice().GetSwapChainSupport();

        vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
        vk::PresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
        vk::Extent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
        if(swapChainSupport.Capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.Capabilities.maxImageCount) {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.surface = RenderSystem::GetDevice().Surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        QueueFamilyIndices indices = RenderSystem::GetDevice().FindPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = { indices.GraphicsFamily, indices.PresentFamily };

        if(indices.GraphicsFamily != indices.PresentFamily) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

        createInfo.presentMode = presentMode;
        createInfo.clipped = true;

        createInfo.oldSwapchain = nullptr;

        if(RenderSystem::GetDevice().GetDevice().createSwapchainKHR(&createInfo, nullptr, &swapChain) != vk::Result::eSuccess) {
            RUI_CORE_ERROR("Failed to create swap chain!");
        }

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        RenderSystem::GetDevice().GetDevice().getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        RenderSystem::GetDevice().GetDevice().getSwapchainImagesKHR(swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void SwapChain::CreateImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for(size_t i = 0; i < swapChainImages.size(); i++) {
            vk::ImageViewCreateInfo viewInfo;
            viewInfo.image = swapChainImages[i];
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if(RenderSystem::GetDevice().GetDevice().createImageView(&viewInfo, nullptr, &swapChainImageViews[i]) != vk::Result::eSuccess) {
                RUI_CORE_ERROR("Failed to create texture image view!");
            }
        }
    }

    void SwapChain::CreateRenderPass() {
        vk::AttachmentDescription depthAttachment;
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = vk::SampleCountFlagBits::e1;
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference depthAttachmentRef;
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentDescription colorAttachment;
        colorAttachment.format = GetSwapChainImageFormat();
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass;
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        vk::SubpassDependency dependency;

        dependency.dstSubpass = 0;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        vk::RenderPassCreateInfo renderPassInfo;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if(RenderSystem::GetDevice().GetDevice().createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
            RUI_CORE_ERROR("Failed to create render pass!");
        }
    }

    void SwapChain::CreateFramebuffers() {
        swapChainFramebuffers.resize(ImageCount());
        for(size_t i = 0; i < ImageCount(); i++) {
            std::array<vk::ImageView, 2> attachments = { swapChainImageViews[i], depthImageViews[i] };

            vk::Extent2D swapChainExtent = GetSwapChainExtent();
            vk::FramebufferCreateInfo framebufferInfo;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if(RenderSystem::GetDevice().GetDevice().createFramebuffer(&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess) {
                RUI_CORE_ERROR("Failed to create framebuffer!");
            }
        }
    }

    void SwapChain::CreateDepthResources() {
        vk::Format depthFormat = FindDepthFormat();
        vk::Extent2D swapChainExtent = GetSwapChainExtent();

        depthImages.resize(ImageCount());
        depthImageMemorys.resize(ImageCount());
        depthImageViews.resize(ImageCount());

        for(int i = 0; i < depthImages.size(); i++) {
            vk::ImageCreateInfo imageInfo;
            imageInfo.imageType = vk::ImageType::e2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = vk::ImageTiling::eOptimal;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;
            imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            imageInfo.samples = vk::SampleCountFlagBits::e1;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            imageInfo.flags = {};

            //RenderSystem::GetDevice().CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,depthImages[i],depthImageMemorys[i]);

            vma::AllocationCreateInfo create_info;
            create_info.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

            vma::AllocationInfo info;
            vma::Allocation alloc;
            RenderSystem::GetDevice().m_Allocator.createImage(&imageInfo, &create_info, &depthImages[i], &alloc, &info);
            
            RUI_CORE_TRACE("Type: {0}, Size: {1}", info.memoryType, info.size);

            vk::ImageViewCreateInfo viewInfo;
            viewInfo.image = depthImages[i];
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if(RenderSystem::GetDevice().GetDevice().createImageView(&viewInfo, nullptr, &depthImageViews[i]) != vk::Result::eSuccess) {
                RUI_CORE_ERROR("Failed to create texture image view!");
            }
        }
    }

    void SwapChain::CreateSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(ImageCount(), nullptr);

        vk::SemaphoreCreateInfo semaphoreInfo;

        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if(RenderSystem::GetDevice().GetDevice().createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
               RenderSystem::GetDevice().GetDevice().createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
               RenderSystem::GetDevice().GetDevice().createFence(&fenceInfo, nullptr, &inFlightFences[i]) != vk::Result::eSuccess) {
            	RUI_CORE_ERROR("failed to create synchronization objects for a frame!");
            }
        }
    }

    vk::SurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for(const auto& availableFormat : availableFormats) {
            if(availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    vk::PresentModeKHR SwapChain::ChooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for(const auto& availablePresentMode : availablePresentModes) {
            if(availablePresentMode == vk::PresentModeKHR::eMailbox) {
                RUI_CORE_TRACE("Present mode: Mailbox");
                return availablePresentMode;
            }
        }

        // for (const auto &availablePresentMode : availablePresentModes) {
        //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        //     std::cout << "Present mode: Immediate" << std::endl;
        //     return availablePresentMode;
        //   }
        // }

        RUI_CORE_TRACE("Present mode: V-Sync");
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D SwapChain::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            vk::Extent2D actualExtent = windowExtent;
            actualExtent.width = std::max(
                capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(
                capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    void SwapChain::ReCreateSwapChain() {
		windowExtent = Application::Get().GetDisplay().GetExtent();
        RUI_CORE_INFO("Window Resized: {0} {1}", windowExtent.width, windowExtent.height);

        RenderSystem::GetDevice().GetDevice().waitIdle();

        for(size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            RenderSystem::GetDevice().GetDevice().destroyFramebuffer(swapChainFramebuffers[i], nullptr);
        }

        RenderSystem::GetDevice().GetDevice().freeCommandBuffers(RenderSystem::GetDevice().GetCommandPool(), static_cast<uint32_t>(RenderSystem::GetData().CommandBuffers.size()), RenderSystem::GetData().CommandBuffers.data());

        RenderSystem::GetDevice().GetDevice().destroyPipeline(RenderSystem::GetData().Pipeline->GetPipeline(), nullptr);
    	RenderSystem::GetDevice().GetDevice().destroyPipelineLayout(RenderSystem::GetData().PipelineLayout, nullptr);
        RenderSystem::GetDevice().GetDevice().destroyRenderPass(renderPass, nullptr);

        for(size_t i = 0; i < swapChainImageViews.size(); i++) {
            RenderSystem::GetDevice().GetDevice().destroyImageView(swapChainImageViews[i], nullptr);
        }


        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        RenderSystem::CreatePipelineLayout();
        RenderSystem::CreatePipeline();
        CreateDepthResources();
        CreateFramebuffers();
        RenderSystem::CreateCommandBuffers();
    }

    vk::Format SwapChain::FindDepthFormat() {
        return RenderSystem::GetDevice().FindSupportedFormat(
        { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }, 
        vk::ImageTiling::eOptimal, 
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }

    std::unique_ptr<SwapChain> SwapChain::Create(vk::Extent2D windowExtent) {
        return std::make_unique<SwapChain>(windowExtent);
    }
}
