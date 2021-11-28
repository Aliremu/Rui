#pragma once

#ifdef RUI_PLATFORM_WINDOWS
#else
	#error Only Supports Windows!
#endif

#ifdef RUI_ENABLE_ASSERTS
	#define RUI_ASSERT(x, ...) { if(!(x)) { RUI_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define RUI_CORE_ASSERT(x, ...) { if(!(x)) { RUI_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define RUI_ASSERT(x, ...)
	#define RUI_CORE_ASSERT(x, ...)
#endif

#define RUI_ENGINE "Rui"
#define RUI_VERSION VK_MAKE_VERSION(1, 0, 0)

#define BIT(x) (1 << x)

namespace Rui {
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}