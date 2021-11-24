#pragma once

#include "Log.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.hpp"
#include "Window.h"

namespace Rui {
	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};

	struct QueueFamilyIndices {
		uint32_t GraphicsFamily;
		uint32_t PresentFamily;
		uint32_t ComputeFamily;

		bool GraphicsFamilyHasValue = false;
		bool PresentFamilyHasValue = false;
		bool ComputeFamilyHasValue = false;

		bool IsComplete() { return GraphicsFamilyHasValue && PresentFamilyHasValue; }
	};

	class Device {
	public:
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		Device();
		~Device();

		inline vk::Device& GetDevice() { return m_Device; }
		inline vk::CommandPool& GetCommandPool() { return m_CommandPool; }
		inline vk::SurfaceKHR& Surface() { return m_Surface; }
		inline vk::Queue& GraphicsQueue() { return m_GraphicsQueue; }
		inline vk::Queue& PresentQueue() { return m_PresentQueue; }

		inline QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(m_PhysicalDevice); }
		inline SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(m_PhysicalDevice); }

		vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		vma::Allocator m_Allocator;

		static std::unique_ptr<Device> Create();
	private:

		vk::Instance m_Instance;
		vk::DebugUtilsMessengerEXT m_DebugMessenger;
		vk::PhysicalDevice m_PhysicalDevice = nullptr;
		vk::CommandPool m_CommandPool;
		//vk::DispatchLoaderDynamic dldy;
		
		vk::Device m_Device;
		vk::SurfaceKHR m_Surface;
		vk::Queue m_GraphicsQueue;
		vk::Queue m_PresentQueue;

		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();

		std::vector<const char*> GetRequiredExtensions();
		QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice& device);
		SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device);

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}

