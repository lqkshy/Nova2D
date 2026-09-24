#include "Game.h"
#include "Logger.h"
#include <iostream>
#include <glm/glm.hpp>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL.h>

Game::Game() {
    isRunning = false;
    Logger::Log("Game constuctor called!");
}

Game::~Game() {
    Logger::Log("Game destructor called!");
}

void Game::Initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        Logger::Err("Error Initializing SDL: ");
        return;
    }

    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(displayID);
    if (displayMode) {
        windowWidth = 800; //displayMode->w;
        windowHeight = 600;//displayMode->h;
    }

    window = SDL_CreateWindow("Nova2D", windowWidth, windowHeight, SDL_WINDOW_BORDERLESS);
    if (!window) {
        Logger::Err("Error creating SDL window: ");
        return;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        Logger::Err("Error creating SDL renderer: ");
        return;
    }

    SDL_SetWindowFullscreen(window, true);
    isRunning = true;
}

void Game::Run() {
    Setup();
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

glm::vec2 playerPosition;
glm::vec2 playerVelocity;

void Game::Setup() {
    playerPosition = glm::vec2(10.0, 20.0);
    playerVelocity = glm::vec2(10.0, 5.0);
}

void Game::Update() {
    Uint64 timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
        SDL_Delay(static_cast<Uint32>(timeToWait));  
    }

    float deltaTime = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0f;
    millisecsPreviousFrame = SDL_GetTicks();

    playerPosition.x += playerVelocity.x * deltaTime;
    playerPosition.y += playerVelocity.y * deltaTime;
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 0, 255);
    SDL_RenderClear(renderer);

    // Load a PNG texture
    SDL_Surface* surface = IMG_Load("./assets/images/tank-tiger-right.png");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    // What is the destination rectangle that we want to place our texture
    SDL_FRect dstRect = {
        playerPosition.x,
        playerPosition.y,
        32, 32 
    };
    SDL_RenderTexture(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}
 
void Game::Destroy() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}