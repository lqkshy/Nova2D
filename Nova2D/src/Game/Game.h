#ifndef GAME_H
#define GAME_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#include <SDL3/SDL.h>
#include <sol/sol.hpp>
#include <memory>

constexpr int    FPS                 = 60;
constexpr Uint64 MILLISECS_PER_FRAME = 1000 / FPS;
constexpr double MAX_DELTA_TIME      = 0.05;   // 50 ms: protects against big jumps after a lag spike

class Game {
    private:
        bool isRunning = false;
        bool isDebug = false;
        bool vsyncEnabled = false;
        Uint64 millisecsPreviousFrame = 0;
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Rect camera{};

        sol::state lua;

        std::unique_ptr<Registry> registry;
        std::unique_ptr<AssetStore> assetStore;
        std::unique_ptr<EventBus> eventBus;

    public:
        Game();
        ~Game();
        void Initialize();
        void Run();
        void Setup();
        void ProcessInput();
        void Update();
        void Render();
        void Destroy();

        static int windowWidth;
        static int windowHeight;
        static int mapWidth;
        static int mapHeight;
};

#endif
