#include "Application.h"

using Clock = std::chrono::steady_clock;
using namespace std::literals;

namespace Rui {

	Application* Application::Instance = nullptr;

	Application::Application(const std::string& title, int w, int h) {
		Rui::Log::Init(title);
		RUI_CORE_INFO("Creating Logger!");

		Instance = this;
		m_Window = Window::Create(title, w, h);
		m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		RenderSystem::Init();
	}

	Application::~Application() {
		RUI_CORE_INFO("Destroying Application!");
	}

	void Application::Run() {
		const int tps = 75;
		auto constexpr dt = std::chrono::duration<long long, std::ratio<1, tps>>{ 1 };
		using duration = decltype(Clock::duration{} + dt);
		using time_point = std::chrono::time_point<Clock, duration>;

		time_point t{};

		time_point currentTime = Clock::now();
		duration accumulator = 0s;

		while(m_Running) {
			m_Window->OnUpdate();
			time_point newTime = Clock::now();
			auto frameTime = newTime - currentTime;
			if(frameTime > 250ms)
				frameTime = 250ms;
			currentTime = newTime;

			accumulator += frameTime;

			Timestep ts((double) newTime.time_since_epoch().count(), 1.0f / tps, 0.0);

			while(accumulator >= dt) {
				m_Scene->OnUpdate(ts);

				t += dt;
				accumulator -= dt;
			}

			const double alpha = std::chrono::duration<double>{ accumulator } / dt;

			//State state = currentState * alpha + previousState * (1 - alpha);
			ts.m_Interpolation = alpha;
			m_Scene->OnRender(ts);
		}

		RenderSystem::GetDevice().GetDevice().waitIdle();
	}
	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowClosedEvent>(std::bind(&Application::OnWindowClose, this, std::placeholders::_1));
		dispatcher.Dispatch<WindowResizedEvent>([&](WindowResizedEvent& ev) -> bool {
			RUI_CORE_INFO("Window Resized: {0} {1}", ev.GetWidth(), ev.GetHeight());

			RenderSystem::GetSwapChain().ReCreateSwapChain();

			return true;
		});

		switch(e.GetEventType()) {
		case EventType::KeyPressed: {
			KeyPressedEvent* key = (KeyPressedEvent*)&e;
			if(key->GetKeyCode() == SDLK_q) {
				m_Running = false;
			}

			if(key->GetKeyCode() == SDLK_ESCAPE) {
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
			break;
		}
		}
	}

	bool Application::OnWindowClose(WindowClosedEvent& e) {
		m_Running = false;
		return true;
	}
}
