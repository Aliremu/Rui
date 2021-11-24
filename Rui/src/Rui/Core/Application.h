#pragma once
#include "Core.h"
#include "Device.h"
#include "Log.h"
#include "Scene.h"
#include "SwapChain.h"
#include "Window.h"
#include "Timestep.h"

#include "Rui/Events/WindowEvent.h"
#include "Rui/Events/KeyEvent.h"

#include "Rui/Render/RenderSystem.h"

namespace Rui {
	class Application {
	public:
		Application(const std::string& title, int w, int h);
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		inline Window& GetDisplay() { return *m_Window; }
		inline static Application& Get() { return *Instance; }
		inline void LoadScene(Scene* scene) {
			if(m_Scene) m_Scene->OnUnload();

			m_Scene = scene;
			m_Scene->OnLoad();
		}

	private:
		std::unique_ptr<Window> m_Window;

		Scene* m_Scene = nullptr;

		bool m_Running = true;

		bool OnWindowClose(WindowClosedEvent& e);

		static Application* Instance;
		friend int ::main(int argc, char* argv[]);
	};

	Application* CreateApplication();
}

