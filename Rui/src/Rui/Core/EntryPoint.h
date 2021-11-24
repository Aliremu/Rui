#pragma once

extern Rui::Application* Rui::CreateApplication();

int main(int argc, char** argv) {
	auto app = Rui::CreateApplication();
	app->Run();

	delete app;
	
	return 0;
}