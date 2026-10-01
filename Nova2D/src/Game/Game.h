#ifndef GAME_H
#define GAME_H

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../EventBus/EventBus.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <sol/sol.hpp>
#include <memory>
#include <string>

constexpr int    FPS                 = 60;
constexpr Uint64 MILLISECS_PER_FRAME = 1000 / FPS;
constexpr double MAX_DELTA_TIME      = 0.05;   // 50 ms: protects against big jumps after a lag spike

constexpr int MAX_LEVELS        = 3;      // Level1.lua ... Level3.lua
constexpr int START_LIVES       = 3;
constexpr int SCORE_PER_ENEMY   = 100;
constexpr int LEVEL_CLEAR_BONUS = 500;

enum class GameState {
    Menu,
    Controls,
    Playing,
    Paused,
    PlayerDown,      // the helicopter was shot down, but there are lives left
    LevelComplete,
    GameOver,
    Victory
};

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
        std::unique_ptr<AssetStore> assetStore;   // textures/fonts of the current level
        std::unique_ptr<AssetStore> uiAssets;     // textures used by the menu
        std::unique_ptr<EventBus> eventBus;

        // UI fonts
        TTF_Font* titleFont = nullptr;
        TTF_Font* menuFont = nullptr;
        TTF_Font* hudFont = nullptr;
        TTF_Font* smallFont = nullptr;

        // Game flow
        GameState state = GameState::Menu;
        Uint64 stateChangedAt = 0;
        Uint64 levelStartTime = 0;
        int menuIndex = 0;
        float mouseX = 0.0f;
        float mouseY = 0.0f;

        int currentLevel = 1;
        int score = 0;
        int scoreAtLevelStart = 0;
        int lives = START_LIVES;
        int highScore = 0;
        int enemiesTotal = 0;
        int enemiesLeft = 0;
        std::string levelName;

        // Helpers
        void SetState(GameState newState);
        bool StartLevel(int level);
        void NewGame();
        void BeginLevel(int level);
        int  CountEnemies() const;
        bool LevelExists(int level) const;
        void LoadHighScore();
        void UpdateHighScore();

        void OnKeyDown(SDL_Keycode key);
        void OnClick();
        void ActivateMenuItem(int index);
        SDL_FRect MenuButtonRect(int index) const;

        void RenderMenu();
        void RenderControls();
        void RenderWorld();
        void RenderHUD();
        void RenderOverlay();

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
