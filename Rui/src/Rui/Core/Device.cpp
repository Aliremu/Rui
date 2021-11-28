#define VMA_IMPLEMENTATION
#include "Device.h"

#include "Application.h"

namespace Rui {
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		RUI_CORE_ERROR("Validation Layer: {0}", pCallbackData->pMessage);

		return VK_FALSE;
	}

	vk::Result CreateDebugUtilsMessengerEXT(const vk::Instance& instance, const vk::DebugUtilsMessengerCreateInfoEXT& pCreateInfo, const vk::AllocationCallbacks *pAllocator, vk::DebugUtilsMessengerEXT *pDebugMessenger) {
		vk::DispatchLoaderDynamic dldy(instance, vkGetInstanceProcAddr);
		auto result = instance.createDebugUtilsMessengerEXT(&pCreateInfo, pAllocator, pDebugMessenger, dldy);

		return result;
	}

	void DestroyDebugUtilsMessengerEXT(const vk::Instance& instance, vk::DebugUtilsMessengerEXT debugMessenger, const vk::AllocationCallbacks* pAllocator) {
		vk::DispatchLoaderDynamic dldy(instance, vkGetInstanceProcAddr);
		instance.destroyDebugUtilsMessengerEXT(debugMessenger, pAllocator, dldy);
	}

	void PopulateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagsEXT(/*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |*/VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
		createInfo.messageType = vk::DebugUtilsMessageTypeFlagsEXT(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.pUserData = nullptr;
	}


	Device::Device() {
		RUI_CORE_INFO("Creating Device!");
		
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();

		vma::AllocatorCreateInfo create_info({}, m_PhysicalDevice, m_Device);
		vma::createAllocator(&create_info, &m_Allocator);
	}

	Device::~Device() {
		//m_Device.waitIdle();
		if(enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}
	}

	void Device::CreateInstance() {
		vk::ApplicationInfo app_info(
			Application::Get().GetDisplay().GetTitle().c_str(),
			VK_MAKE_VERSION(1, 0, 0),
			RUI_ENGINE_NAME,
			RUI_ENGINE_VERSION,
			VK_API_VERSION_1_2
		);

		std::vector<const char*> extensions = GetRequiredExtensions();

		vk::InstanceCreateInfo instance_create_info({}, &app_info);
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instance_create_info.ppEnabledExtensionNames = extensions.data();

		if(enableValidationLayers) {
			instance_create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			instance_create_info.ppEnabledLayerNames = validationLayers.data();

			vk::DebugUtilsMessengerCreateInfoEXT debug_create_info;
			PopulateDebugMessengerCreateInfo(debug_create_info);
			instance_create_info.pNext = &debug_create_info;
		} else {
			instance_create_info.enabledLayerCount = 0;
			instance_create_info.pNext = nullptr;
		}

		auto result = vk::createInstance(instance_create_info);

		RUI_CORE_ASSERT(result.result == vk::Result::eSuccess, "Failed to create Vulkan Instance!");

		m_Instance = result.value;
	}

	void Device::SetupDebugMessenger() {
		if(!enableValidationLayers) return;
		vk::DebugUtilsMessengerCreateInfoEXT create_info;
		PopulateDebugMessengerCreateInfo(create_info);
		if(CreateDebugUtilsMessengerEXT(m_Instance, create_info, nullptr, &m_DebugMessenger) != vk::Result::eSuccess) {
			RUI_CORE_ERROR("Failed to set up debug messenger!");
		}
	}

	void Device::CreateSurface() {
		Application::Get().GetDisplay().CreateSurface(m_Instance, &m_Surface);
	}

	void Device::PickPhysicalDevice() {
		uint32_t count = 0;
		m_Instance.enumeratePhysicalDevices(&count, nullptr);
		std::vector<vk::PhysicalDevice> devices(count);
		m_Instance.enumeratePhysicalDevices(&count, devices.data());

		for(auto& it : devices) {
			RUI_CORE_TRACE("{0}", it.getProperties().deviceName);
		}

		m_PhysicalDevice = devices[0];
		RUI_CORE_TRACE("Chosen Device: {0} - {1}", m_PhysicalDevice.getProperties().deviceID, m_PhysicalDevice.getProperties().deviceName);
	}

	void Device::CreateLogicalDevice() {
		QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

		std::vector<vk::DeviceQueueCreateInfo> queue_create_info;
		std::set<uint32_t> unique_queue_families = { indices.GraphicsFamily, indices.PresentFamily };

		float queue_priority = 1.0f;
		for(uint32_t queue_family : unique_queue_families) {
			vk::DeviceQueueCreateInfo create_info({}, queue_family, 1, &queue_priority);
			queue_create_info.push_back(create_info);
		}

		vk::PhysicalDeviceFeatures device_features;
		device_features.samplerAnisotropy = true;

		vk::DeviceCreateInfo create_info;
		create_info.queueCreateInfoCount	= static_cast<uint32_t>(queue_create_info.size());
		create_info.pQueueCreateInfos		= queue_create_info.data();

		create_info.pEnabledFeatures	    = &device_features;
		create_info.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
		create_info.ppEnabledExtensionNames = deviceExtensions.data();

		if(enableValidationLayers) {
			create_info.enabledLayerCount	= static_cast<uint32_t>(validationLayers.size());
			create_info.ppEnabledLayerNames = validationLayers.data();
		} else {
			create_info.enabledLayerCount = 0;
		}

		m_PhysicalDevice.createDevice(&create_info, nullptr, &m_Device);

		m_Device.getQueue(indices.GraphicsFamily, 0, &m_GraphicsQueue);
		m_Device.getQueue(indices.PresentFamily,  0, &m_PresentQueue);
	}

	void Device::CreateCommandPool() {
		QueueFamilyIndices queue_family_indices = FindQueueFamilies(m_PhysicalDevice);

		vk::CommandPoolCreateInfo pool_create_info(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queue_family_indices.GraphicsFamily);

		auto result = m_Device.createCommandPool(&pool_create_info, nullptr, &m_CommandPool);
		RUI_CORE_ASSERT(result == vk::Result::eSuccess, "Failed to create command pool!");
	}

	std::vector<const char*> Rui::Device::GetRequiredExtensions() {
		uint32_t count = 0;
		SDL_Vulkan_GetInstanceExtensions(nullptr, &count, nullptr);
		std::vector<const char*> extensions(count);
		SDL_Vulkan_GetInstanceExtensions(nullptr, &count, extensions.data());

		if(enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		for(auto& a : extensions) {
			RUI_CORE_TRACE("Extension: {0}", a);
		}

		return extensions;
	}

	QueueFamilyIndices Device::FindQueueFamilies(const vk::PhysicalDevice& device) {
		QueueFamilyIndices indices;

		auto queue_families = device.getQueueFamilyProperties();

		int i = 0;
		for(const auto& queue_family : queue_families) {
			if(queue_family.queueCount > 0 && queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
				indices.GraphicsFamily = i;
				indices.GraphicsFamilyHasValue = true;
			}
			//bool presentSupport = false;
			vk::Bool32 presentSupport;
			device.getSurfaceSupportKHR(i, m_Surface, &presentSupport);
			if(queue_family.queueCount > 0 && presentSupport) {
				indices.PresentFamily = i;
				indices.PresentFamilyHasValue = true;
			}

			if(queue_family.queueCount > 0 && queue_family.queueFlags & vk::QueueFlagBits::eCompute) {
				indices.ComputeFamily = i;
				indices.ComputeFamilyHasValue = true;
			}

			if(indices.IsComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails Device::QuerySwapChainSupport(const vk::PhysicalDevice& device) {
		SwapChainSupportDetails details;
		device.getSurfaceCapabilitiesKHR(m_Surface, &details.Capabilities);

		uint32_t format_count = 0;
		device.getSurfaceFormatsKHR(m_Surface, &format_count, nullptr);

		if(format_count != 0) {
			details.Formats.resize(format_count);
			device.getSurfaceFormatsKHR(m_Surface, &format_count, details.Formats.data());
		}

		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &present_mode_count, nullptr);

		if(present_mode_count != 0) {
			details.PresentModes.resize(present_mode_count);
			device.getSurfacePresentModesKHR(
				m_Surface,
				&present_mode_count,
				details.PresentModes.data());
		}

		return details;
	}

	vk::Format Device::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
		for(vk::Format format : candidates) {
			vk::FormatProperties props;
			m_PhysicalDevice.getFormatProperties(format, &props);

			if(tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			} else if(tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		RUI_CORE_ERROR("Failed to find supported format!");
		return vk::Format::eUndefined;
	}

	uint32_t Device::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		vk::PhysicalDeviceMemoryProperties memProperties;
		m_PhysicalDevice.getMemoryProperties(&memProperties);
		for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		RUI_CORE_ERROR("Failed to find suitable memory type!");
		return -1;
	}

	std::unique_ptr<Device> Device::Create() {
		return std::make_unique<Device>();
	}
}
