#include "./Game.h"
#include "./LevelLoader.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderHealthBarSystem.h"
#include "../Systems/RenderGUISystem.h"
#include "../Systems/ScriptSystem.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlrenderer3.h>

int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

Game::Game() {
    isRunning = false;
    isDebug = false;
    registry = std::make_unique<Registry>();
    assetStore = std::make_unique<AssetStore>();
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called!");
}

Game::~Game() {
    Logger::Log("Game destructor called!");   
}

void Game::Initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) { Logger::Err("Error initializing SDL."); return; }
    if (!TTF_Init()) { Logger::Err("Error initializing SDL TTF."); return; }

    // Fixed logical resolution: the game always draws at 1280x720 and SDL scales it to any screen
    windowWidth = 1280;
    windowHeight = 720;

    window = SDL_CreateWindow("Nova2D", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) { Logger::Err("Error creating SDL window."); return; }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) { Logger::Err("Error creating SDL renderer."); return; }

    vsyncEnabled = SDL_SetRenderVSync(renderer, 1);
    if (!vsyncEnabled) { Logger::Log("VSync not available, using manual frame cap."); }
    SDL_SetRenderLogicalPresentation(renderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    camera = { 0, 0, windowWidth, windowHeight };
    isRunning = true;
}

void Game::ProcessInput() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        ImGui_ImplSDL3_ProcessEvent(&sdlEvent);

        switch (sdlEvent.type) {
        case SDL_EVENT_QUIT:
            isRunning = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (sdlEvent.key.key == SDLK_ESCAPE) isRunning = false;
            if (sdlEvent.key.key == SDLK_F1) isDebug = !isDebug;
            if (sdlEvent.key.key == SDLK_F11) {
                bool isFullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
                SDL_SetWindowFullscreen(window, !isFullscreen);
            }
            eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.key);
            break;
        }
    }
}

void Game::Setup() {
    // Add the sytems that need to be processed in our game
    registry->AddSystem<MovementSystem>();
    registry->AddSystem<RenderSystem>();
    registry->AddSystem<AnimationSystem>();
    registry->AddSystem<CollisionSystem>();
    registry->AddSystem<RenderColliderSystem>();
    registry->AddSystem<DamageSystem>();
    registry->AddSystem<KeyboardControlSystem>();
    registry->AddSystem<CameraMovementSystem>();
    registry->AddSystem<ProjectileEmitSystem>();
    registry->AddSystem<ProjectileLifecycleSystem>();
    registry->AddSystem<RenderTextSystem>();
    registry->AddSystem<RenderHealthBarSystem>();
    registry->AddSystem<RenderGUISystem>();
    registry->AddSystem<ScriptSystem>();

    // Create the bindings between C++ and Lua
    registry->GetSystem<ScriptSystem>().CreateLuaBindings(lua);

    // Load the first level
    LevelLoader loader;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
    loader.LoadLevel(lua, registry, assetStore, renderer, 2);

    // Subscribe to events once (systems live for the whole game)
    registry->GetSystem<MovementSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(eventBus);

    // Start the clock here so the first frame does not get a huge deltaTime
    millisecsPreviousFrame = SDL_GetTicks();
}

void Game::Update() {
    Uint64 now = SDL_GetTicks();
    double deltaTime = (now - millisecsPreviousFrame) / 1000.0;
    millisecsPreviousFrame = now;
    if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;

    // Update the registry to process the entities that are waiting to be created/deleted
    registry->Update();
    
    // Invoke all the systems that need to update 
    registry->GetSystem<MovementSystem>().Update(deltaTime);
    registry->GetSystem<AnimationSystem>().Update();
    registry->GetSystem<CollisionSystem>().Update(eventBus);
    registry->GetSystem<ProjectileEmitSystem>().Update(registry);
    registry->GetSystem<CameraMovementSystem>().Update(camera);
    registry->GetSystem<ProjectileLifecycleSystem>().Update();
    registry->GetSystem<ScriptSystem>().Update(deltaTime, static_cast<int>(now));
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    // Invoke all the systems that need to render 
    registry->GetSystem<RenderSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderTextSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderHealthBarSystem>().Update(renderer, assetStore, camera);
    if (isDebug) {
        registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
        registry->GetSystem<RenderGUISystem>().Update(renderer, registry, camera);
    }

    SDL_RenderPresent(renderer);
}

void Game::Run() {
    Setup();
    while (isRunning) {
        ProcessInput();
        Update();
        Render();

        // Manual frame cap only if VSync is not available
        if (!vsyncEnabled) {
            Uint64 frameTime = SDL_GetTicks() - millisecsPreviousFrame;
            if (frameTime < MILLISECS_PER_FRAME) SDL_Delay(static_cast<Uint32>(MILLISECS_PER_FRAME - frameTime));
        }
    }
}

void Game::Destroy() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
