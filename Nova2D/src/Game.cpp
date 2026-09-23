#include "Game.h"
#include <iostream>
#include <SDL3/SDL.h>

Game::Game() {
    isRunning = false;
    std::cout << "Game constructor called!" << std::endl;
}

Game::~Game() {
    std::cout << "Game destructor called!" << std::endl;
}

void Game::Initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "Error Initializing SDL: " << SDL_GetError() << std::endl;
        return;
    }

    window = SDL_CreateWindow("Nova2D", 800, 600, SDL_WINDOW_BORDERLESS);
    if (!window) {
        std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
        return;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "Error creating SDL renderer: " << SDL_GetError() << std::endl;
        return;
    }

    isRunning = true;
}

void Game::Run() {
    while (isRunning) {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::ProcessInput() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {       
        switch (sdlEvent.type) {
        case SDL_EVENT_QUIT:                
            isRunning = false;
            break;
        case SDL_EVENT_KEY_DOWN:             
            if (sdlEvent.key.key == SDLK_ESCAPE) {  
                isRunning = false;
            }
            break;
        }
    }
}

void Game::Update() {
    // TODO: Update game objects...
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);

    // TODO: Render game objects...

    SDL_RenderPresent(renderer);
}

void Game::Destroy() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}