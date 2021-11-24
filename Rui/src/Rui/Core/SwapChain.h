#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

namespace Rui {

    class SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        
        SwapChain(vk::Extent2D windowExtent);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        void operator=(const SwapChain&) = delete;

        vk::Framebuffer GetFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        vk::RenderPass GetRenderPass() { return renderPass; }
        vk::ImageView GetImageView(int index) { return swapChainImageViews[index]; }
        size_t ImageCount() { return swapChainImages.size(); }
        vk::Format GetSwapChainImageFormat() { return swapChainImageFormat; }
        vk::Extent2D GetSwapChainExtent() { return swapChainExtent; }

        void ReCreateSwapChain();

        uint32_t Width() { return swapChainExtent.width; }
        uint32_t Height() { return swapChainExtent.height; }

        float ExtentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        vk::Format FindDepthFormat();

        vk::Result AcquireNextImage(uint32_t* imageIndex);
        vk::Result SubmitCommandBuffers(const vk::CommandBuffer* buffers, uint32_t* imageIndex);

		static std::unique_ptr<SwapChain> Create(vk::Extent2D windowExtent);
    private:
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        vk::PresentModeKHR ChooseSwapPresentMode(
            const std::vector<vk::PresentModeKHR>& availablePresentModes);
        vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;

        std::vector<vk::Framebuffer> swapChainFramebuffers;
        vk::RenderPass renderPass;

        std::vector<vk::Image> depthImages;
        std::vector<vk::DeviceMemory> depthImageMemorys;
        std::vector<vk::ImageView> depthImageViews;
        std::vector<vk::Image> swapChainImages;
        std::vector<vk::ImageView> swapChainImageViews;

        vk::Extent2D windowExtent;

        vk::SwapchainKHR swapChain;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;
        std::vector<vk::Fence> imagesInFlight;
        size_t currentFrame = 0;
    };

}
