#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Rui {
	class Log {
	public:
		static void Init(const std::string& title);

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#define RUI_CORE_TRACE(...) ::Rui::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RUI_CORE_INFO(...)  ::Rui::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RUI_CORE_WARN(...)  ::Rui::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RUI_CORE_ERROR(...) ::Rui::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RUI_CORE_FATAL(...) ::Rui::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define RUI_TRACE(...) ::Rui::Log::GetClientLogger()->trace(__VA_ARGS__)
#define RUI_INFO(...)  ::Rui::Log::GetClientLogger()->info(__VA_ARGS__)
#define RUI_WARN(...)  ::Rui::Log::GetClientLogger()->warn(__VA_ARGS__)
#define RUI_ERROR(...) ::Rui::Log::GetClientLogger()->error(__VA_ARGS__)
#define RUI_FATAL(...) ::Rui::Log::GetClientLogger()->critical(__VA_ARGS__)
