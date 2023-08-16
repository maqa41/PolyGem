#include <iostream>
#include "SDL.h"
#include "SDL_ttf.h"
#undef main

#include "benchmark.h"
#include "core.h"
#include "gui.h"
#include "core_functions.h"


plg::Vec2 winResolution(1076.0f, 620.0f);

SDL_Renderer* renderer;
SDL_Window* window;
bool fullscreen;
bool playSim = false;

int main() {
	gui::GUIEvent guiEvent;

	fullscreen = false;
	int flags = 0;
	flags = SDL_WINDOW_RESIZABLE;

	if (fullscreen) {
		flags = flags | SDL_WINDOW_FULLSCREEN;
	}
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		std::cout << "Subsystems Initialized!\n";

		window = SDL_CreateWindow("Test Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)winResolution.x, (int)winResolution.y, flags);
		if (window) {
			std::cout << "Window Created!\n";
			SDL_SetWindowMinimumSize(window, 100, 100);
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer) {
			SDL_SetRenderDrawColor(renderer, 36, 36, 36, 255);
			std::cout << "Renderer created!\n";
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			*guiEvent.GetQuitState() = false;
		}
		TTF_Init();
	}
	
	gui::InitializeGUIStatics(renderer);
	//gui::Label testLabel(renderer, {100, 100, 0, 0}, "Hello, Label!", 20, {255, 255, 0 , 0}, {36, 36, 36, SDL_ALPHA_OPAQUE});
	//gui::Button testButton(renderer, { 250, 200, 0, 0 }, "Hello, Button", 20);
	//gui::CheckButton testCheck(renderer, { 400, 200, 0, 0 }, "Button? Duh?!", 20, { 150, 230, 90 });
	//gui::RadioButton testRadio(renderer, { 400, 250, 0, 0 }, { "radio1", "radio2", "radio3", "Radio, Duh?!" }, 20, gui::DefaultPrimaryButtonColor);
	gui::Layer testLayer({ 0, 0 });
	testLayer.AddButton(renderer, { 250, 200, 0, 0 }, "Hello, Button", 20);
	testLayer.AddSlider(renderer, { 100, 400, 0, 0 }, "GOD", 20, 150, GUI_HORIZONTAL, gui::DefaultDestructiveButtonColor);
	testLayer.AddSlider(renderer, { 300, 400, 0, 0 }, "ZEUS", 20, 150, GUI_VERTICAL, gui::DefaultDestructiveButtonColor);
	testLayer.AddCheckButton(renderer, { 400, 200, 0, 0 }, "Button? Duh?!", 20, gui::DefaultPrimaryButtonColor);
	testLayer.AddCheckButton(renderer, { 400, 240, 0, 0 }, "Button? Duh?!", 20, gui::DefaultPrimaryButtonColor);
	testLayer.AddRadioButton(renderer, { 400, 280, 0, 0 }, { "radio1", "radio2", "radio3", "Radio, Duh?!" }, 20, gui::DefaultPrimaryButtonColor);
	//testCheck.SetState();
	while (!(*guiEvent.GetQuitState())) {

		SDL_SetRenderDrawColor(renderer, 36, 36, 36, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		gui::RetriveGUIEvents(&guiEvent);
		gui::HandleGUIEvents(&guiEvent, &testLayer);

		testLayer.Render(renderer, { 0, 0 });
		//testLabel.Render(renderer, { 20, 20 });
		//testButton.Render(renderer, { 20, 20 });
		//testCheck.Render(renderer, { 50, 0 });
		//testRadio.Render(renderer, { 0, 0 });
		SDL_RenderPresent(renderer);

		SDL_Delay(50);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	TTF_Quit();

	return 0;
}