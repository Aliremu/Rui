#pragma once
#include "Timestep.h"
#include "Rui/Events/Event.h"

#include <entt/entt.hpp>

namespace Rui {
	class Scene {
	public:
		Scene() = default;
		virtual ~Scene() = default;

		virtual void OnLoad() = 0;
		virtual void OnUnload() = 0;

		virtual void OnUpdate(const Timestep& ts) = 0;
		virtual void OnRender(const Timestep& ts) = 0;
		virtual void OnEvent(Event& event) = 0;

	private:
		entt::registry m_Registry;
	};
}

