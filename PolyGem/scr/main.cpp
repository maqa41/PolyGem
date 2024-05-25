#include <iostream>
#include "SDL.h"
#include "SDL_ttf.h"
#undef main

#include "benchmark.h"
#include "core_scene.h"
#include "gui.h"
#include "core_functions.h"


plg::Vec2 winResolution(1076.0f, 620.0f);

SDL_Renderer* renderer;
SDL_Window* window;
bool fullscreen;

static plg::Vec2 GetCircleCenter(plg::Vec2 left, plg::Vec2 middle, plg::Vec2 right) {
	plg::Vec2 line_1 = (middle - left).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 line_2 = (right - middle).RotateByVec(plg::Vec2(0.0f, 1.0f));
	plg::Vec2 segment_start_1 = (left + middle) * 0.5;
	plg::Vec2 segment_end_1 = segment_start_1 + line_1;
	plg::Vec2 segment_start_2 = (middle + right) * 0.5;
	plg::Vec2 segment_end_2 = segment_start_2 + line_2;

	float t1 = ((segment_start_2.y - segment_end_2.y) * (segment_start_1.x - segment_start_2.x) +
		(segment_end_2.x - segment_start_2.x) * (segment_start_1.y - segment_start_2.y)) /
		((segment_end_2.x - segment_start_2.x) * (segment_start_1.y - segment_end_1.y) -
			(segment_start_1.x - segment_end_1.x) * (segment_end_2.y - segment_start_2.y));
	segment_start_1.AddScaledVec(line_1, t1);
	return segment_start_1;
}

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
	gui::Layer testLayer(renderer, { 10, 10, 200, 100 }, gui::DefaultColorBG);
	gui::Frame testFrame(renderer, { 200, 200, 360, 360 });
	testLayer.AddRadioButton(renderer, { 10, 10, 0, 0 }, { "Vertex Mode", "Edge Mode", "Face Mode"}, 20, gui::DefaultPrimaryButtonColor);
	gui::RadioButton* edgeButton = testLayer.GetRadioButtonIterator().operator->();
	container::List<plg::Mesh> sceneMesh;
	sceneMesh.Append(plg::Mesh({ plg::Vertex(50, 50), plg::Vertex(90, 150), plg::Vertex(130, 100), plg::Vertex(100, 100),
		plg::Vertex(50, 140), plg::Vertex(80, 140), plg::Vertex(20, 90), plg::Vertex(180, 40), plg::Vertex(150, 40),
		plg::Vertex(240, 110), plg::Vertex(190, 130), plg::Vertex(50, 200), plg::Vertex(120, 170), plg::Vertex(200, 240), plg::Vertex(210, 20) }));
	
	gui::TreeView testTree(renderer, { 500, 10, 0, 0 }, { "Branch0", "Branch01", "Branch02", "Branch1", "Branch11", "Branch111", "Branch112", "Branch113", "Branch12", "Branch121", "Branch122", "Branch2" },
		{ 0, 1, 1, 0, 1, 2, 2, 2, 1, 2, 2, 0 }, 20, gui::DefaultGUIColor, gui::DefaultTextColor, gui::DefaultDestructiveButtonColor);
	
	while (!(*guiEvent.GetQuitState())) {
		container::List<plg::Vertex>* vertices = sceneMesh[0].GetVertexList();
		plg::Vec2 center = GetCircleCenter((*vertices)[0], (*vertices)[1], (*vertices)[2]);
		float radius = center.GetDistanceTo((*vertices)[2]);

		SDL_SetRenderDrawColor(renderer, 36, 36, 36, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		
		gui::RetriveGUIEvents(&guiEvent);
		gui::HandleGUIEvents(&guiEvent, &testLayer);
		gui::HandleSceneEvents(&guiEvent, &testFrame, (void*)(&sceneMesh));
		plg::sceneMeshData.SetMode(edgeButton->GetState());
		
		testFrame.SetRenderTarget(renderer);
		sceneMesh[0].Render(renderer, plg::Vec2());
		testFrame.UnSetRenderTarget(renderer);
		testFrame.Render(renderer);
		testLayer.Render(renderer);
		testTree.Render(renderer);
		
		SDL_RenderPresent(renderer);
		SDL_Delay(17);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	TTF_Quit();

	return 0;
}