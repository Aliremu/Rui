#include "Shader.h"

namespace Rui {
	Shader::Shader(const std::string& filePath, ShaderType type) {

	}

	Shader::~Shader() {
		
	}

	Ref<Shader> Shader::CreateShader(const std::string& filePath, ShaderType type) {
	    return CreateRef<Shader>(filePath, type);
	}
}
