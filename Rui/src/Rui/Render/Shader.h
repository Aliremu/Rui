#pragma once

#include <glm/fwd.hpp>
#include <vulkan/vulkan.hpp>

#include "Rui/Core/Core.h"

namespace Rui {
	enum class ShaderType {
		None = 0,
		Vertex = VK_SHADER_STAGE_VERTEX_BIT,
		Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
		Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
		Compute = VK_SHADER_STAGE_COMPUTE_BIT
	};

	enum class AttribType {
		Uint = 0,
		Int = 1,
		Float = 2,
		Char = 3
	};

	struct Attribute {
		glm::u32 location;
		glm::u32 columns;
		glm::u32 vec_size;
		glm::u32 component_size;
		AttribType type;
	};
	
	class Shader {
	public:
		Shader(const std::string& filePath, ShaderType type);
		virtual ~Shader();

		static Ref<Shader> CreateShader(const std::string& filePath, ShaderType type);

	private:
		vk::ShaderModule m_Handle;

		ShaderType m_Type = ShaderType::None;
		std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> m_Bindings;
		std::vector<vk::PushConstantRange> m_PushConstants;
		std::vector<Attribute> m_Attributes;
		//std::vector<vk::SpecializationMapEntry> _spec_constants;
	};
}

