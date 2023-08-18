#include <iostream>
#include "SDL.h"
#include "SDL_ttf.h"
#undef main

#include "benchmark.h"
#include "core.h"
#include "core_scene.h"
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
	gui::Layer testLayer(renderer, { 10, 10, 150, 100 }, gui::DefaultColorBG);
	testLayer.AddCheckButton(renderer, { 10, 10, 0, 0 }, "Edge mode", 20, gui::DefaultPrimaryButtonColor);
	gui::CheckButton* edgeButton = testLayer.GetCheckButtonIterator().operator->();
	plg::Mesh testMesh({ plg::Vertex(200, 200), plg::Vertex(230, 200), plg::Vertex(240, 220),
		plg::Vertex(250, 260), plg::Vertex(270, 230), plg::Vertex(200, 200), plg::Vertex(170, 170), plg::Vertex(220, 180) });
	
	while (!(*guiEvent.GetQuitState())) {

		SDL_SetRenderDrawColor(renderer, 36, 36, 36, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		
		gui::RetriveGUIEvents(&guiEvent);
		gui::HandleGUIEvents(&guiEvent, &testLayer);
		testMesh.SetMode((edgeButton->GetState()) ? plg::MeshMode::PLG_EDGE : plg::MeshMode::PLG_VERTEX);
		
		testMesh.Render(renderer, plg::Vec2());
		testLayer.Render(renderer);
		
		SDL_RenderPresent(renderer);
		SDL_Delay(50);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	TTF_Quit();

	return 0;
}