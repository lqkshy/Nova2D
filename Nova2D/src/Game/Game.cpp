#include "./Game.h"
#include "./LevelLoader.h"
#include "./UI.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/HealthComponent.h"
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
#include "../Systems/PatrolSystem.h"
#include "../Systems/EnemyAISystem.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlrenderer3.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

namespace {
    constexpr SDL_Color WHITE  = { 255, 255, 255, 255 };
    constexpr SDL_Color YELLOW = { 255, 210, 60, 255 };
    constexpr SDL_Color RED    = { 255, 90, 90, 255 };
    constexpr SDL_Color GREEN  = { 100, 235, 130, 255 };
    constexpr SDL_Color GREY   = { 170, 180, 195, 255 };
    constexpr SDL_Color DARK   = { 20, 22, 32, 255 };

    const char* MENU_ITEMS[] = { "PLAY", "CONTROLS", "QUIT" };
    constexpr int MENU_COUNT = 3;
    const char* HIGHSCORE_FILE = "./highscore.txt";
    constexpr Uint64 OVERLAY_INPUT_DELAY_MS = 500;   // ignore keys right after a screen appears
}

////////////////////////////////////////////////////////////////////////////////
// Construction / destruction
////////////////////////////////////////////////////////////////////////////////
Game::Game() {
    isRunning = false;
    isDebug = false;
    registry = std::make_unique<Registry>();
    assetStore = std::make_unique<AssetStore>();
    uiAssets = std::make_unique<AssetStore>();
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called!");
}

Game::~Game() {
    Logger::Log("Game destructor called!");
}

void Game::Initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { Logger::Err("Error initializing SDL."); return; }
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) { Logger::Log("Audio not available, the game will run without sound."); }
    if (!TTF_Init()) { Logger::Err("Error initializing SDL TTF."); return; }

    // Fixed logical resolution: the game always draws at 1280x720 and SDL scales it to any screen
    windowWidth = 1280;
    windowHeight = 720;

    window = SDL_CreateWindow("Nova2D", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    if (!window) { Logger::Err("Error creating SDL window."); return; }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) { Logger::Err("Error creating SDL renderer."); return; }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);   // needed for transparent panels
    vsyncEnabled = SDL_SetRenderVSync(renderer, 1);
    if (!vsyncEnabled) { Logger::Log("VSync not available, using manual frame cap."); }
    SDL_SetRenderLogicalPresentation(renderer, windowWidth, windowHeight, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    camera = { 0, 0, windowWidth, windowHeight };

    // Lua: open the libraries and create the C++ <-> Lua bindings only once for the whole game
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
    ScriptSystem().CreateLuaBindings(lua);

    // UI fonts and menu textures
    titleFont = TTF_OpenFont("./assets/fonts/charriot.ttf", 84.0f);
    menuFont  = TTF_OpenFont("./assets/fonts/charriot.ttf", 36.0f);
    hudFont   = TTF_OpenFont("./assets/fonts/charriot.ttf", 20.0f);
    smallFont = TTF_OpenFont("./assets/fonts/charriot.ttf", 16.0f);
    if (!titleFont || !menuFont || !hudFont || !smallFont) {
        Logger::Err("Could not open ./assets/fonts/charriot.ttf (menu text will not show).");
    }
    uiAssets->AddTexture(renderer, "menu-chopper", "./assets/images/chopper-spritesheet.png");

    LoadHighScore();
    isRunning = true;
}

void Game::Destroy() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }

    // Free everything that depends on the renderer BEFORE destroying the renderer
    registry.reset();
    eventBus.reset();
    assetStore->ClearAssets();
    uiAssets->ClearAssets();

    if (titleFont) TTF_CloseFont(titleFont);
    if (menuFont)  TTF_CloseFont(menuFont);
    if (hudFont)   TTF_CloseFont(hudFont);
    if (smallFont) TTF_CloseFont(smallFont);
    titleFont = menuFont = hudFont = smallFont = nullptr;

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;
    TTF_Quit();
    SDL_Quit();
}

////////////////////////////////////////////////////////////////////////////////
// Game flow helpers
////////////////////////////////////////////////////////////////////////////////
void Game::SetState(GameState newState) {
    state = newState;
    stateChangedAt = SDL_GetTicks();
}

bool Game::LevelExists(int level) const {
    return std::filesystem::exists("./assets/scripts/Level" + std::to_string(level) + ".lua");
}

void Game::LoadHighScore() {
    std::ifstream file(HIGHSCORE_FILE);
    if (file) file >> highScore;
}

void Game::UpdateHighScore() {
    if (score > highScore) {
        highScore = score;
        std::ofstream file(HIGHSCORE_FILE);
        if (file) file << highScore;
    }
}

int Game::CountEnemies() const {
    return static_cast<int>(registry->GetEntitiesByGroup("enemies").size());
}

// Builds a brand new world for the given level
bool Game::StartLevel(int level) {
    eventBus = std::make_unique<EventBus>();
    registry = std::make_unique<Registry>();
    assetStore->ClearAssets();

    // Add the systems that need to be processed in our game
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
    registry->AddSystem<PatrolSystem>();
    registry->AddSystem<EnemyAISystem>();

    LevelLoader loader;
    if (!loader.LoadLevel(lua, registry, assetStore, renderer, level)) {
        Logger::Err("Could not load level " + std::to_string(level));
        return false;
    }

    // Move the freshly created entities into their systems
    registry->Update();

    // Subscribe to events once per level
    registry->GetSystem<MovementSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);

    camera = { 0, 0, windowWidth, windowHeight };

    sol::optional<std::string> name = lua["Level"]["name"];
    levelName = name.value_or("Mission " + std::to_string(level));

    currentLevel = level;
    enemiesTotal = CountEnemies();
    enemiesLeft = enemiesTotal;
    levelStartTime = SDL_GetTicks();
    millisecsPreviousFrame = SDL_GetTicks();   // no huge deltaTime on the first frame
    return true;
}

void Game::BeginLevel(int level) {
    if (StartLevel(level)) {
        SetState(GameState::Playing);
    } else {
        SetState(GameState::Menu);
    }
}

void Game::NewGame() {
    score = 0;
    scoreAtLevelStart = 0;
    lives = START_LIVES;
    BeginLevel(1);
}

void Game::ActivateMenuItem(int index) {
    switch (index) {
        case 0: NewGame(); break;
        case 1: SetState(GameState::Controls); break;
        case 2: isRunning = false; break;
    }
}

SDL_FRect Game::MenuButtonRect(int index) const {
    return SDL_FRect{ windowWidth / 2.0f - 190.0f, 345.0f + index * 72.0f, 380.0f, 56.0f };
}

////////////////////////////////////////////////////////////////////////////////
// Input
////////////////////////////////////////////////////////////////////////////////
void Game::OnKeyDown(SDL_Keycode key) {
    if (key == SDLK_F11) {
        bool isFullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
        SDL_SetWindowFullscreen(window, !isFullscreen);
        return;
    }

    const bool isEnter = (key == SDLK_RETURN || key == SDLK_KP_ENTER);
    const bool canUseOverlay = (SDL_GetTicks() - stateChangedAt) > OVERLAY_INPUT_DELAY_MS;

    switch (state) {
        case GameState::Menu:
            if (key == SDLK_UP || key == SDLK_W) menuIndex = (menuIndex + MENU_COUNT - 1) % MENU_COUNT;
            if (key == SDLK_DOWN || key == SDLK_S) menuIndex = (menuIndex + 1) % MENU_COUNT;
            if (isEnter || key == SDLK_SPACE) ActivateMenuItem(menuIndex);
            if (key == SDLK_ESCAPE) isRunning = false;
            break;

        case GameState::Controls:
            if (isEnter || key == SDLK_ESCAPE || key == SDLK_SPACE || key == SDLK_BACKSPACE) SetState(GameState::Menu);
            break;

        case GameState::Playing:
            if (key == SDLK_ESCAPE || key == SDLK_P) SetState(GameState::Paused);
            if (key == SDLK_F1) isDebug = !isDebug;
            break;

        case GameState::Paused:
            if (key == SDLK_ESCAPE || key == SDLK_P) {
                millisecsPreviousFrame = SDL_GetTicks();
                SetState(GameState::Playing);
            }
            if (key == SDLK_Q) SetState(GameState::Menu);
            if (key == SDLK_F1) isDebug = !isDebug;
            break;

        case GameState::PlayerDown:
            if (isEnter && canUseOverlay) {
                score = scoreAtLevelStart;   // retry the level from its starting score
                BeginLevel(currentLevel);
            }
            break;

        case GameState::LevelComplete:
            if (isEnter && canUseOverlay) {
                scoreAtLevelStart = score;
                BeginLevel(currentLevel + 1);
            }
            break;

        case GameState::GameOver:
        case GameState::Victory:
            if ((isEnter || key == SDLK_ESCAPE) && canUseOverlay) SetState(GameState::Menu);
            break;
    }
}

void Game::OnClick() {
    switch (state) {
        case GameState::Menu:
            for (int i = 0; i < MENU_COUNT; i++) {
                if (UI::Contains(MenuButtonRect(i), mouseX, mouseY)) {
                    menuIndex = i;
                    ActivateMenuItem(i);
                    break;
                }
            }
            break;
        case GameState::Controls:
            SetState(GameState::Menu);
            break;
        case GameState::PlayerDown:
        case GameState::LevelComplete:
        case GameState::GameOver:
        case GameState::Victory:
            OnKeyDown(SDLK_RETURN);
            break;
        default:
            break;
    }
}

void Game::ProcessInput() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        ImGui_ImplSDL3_ProcessEvent(&sdlEvent);

        if (sdlEvent.type == SDL_EVENT_QUIT) {
            isRunning = false;
            continue;
        }

        // Convert the mouse position from window pixels to our 1280x720 game coordinates
        SDL_ConvertEventToRenderCoordinates(renderer, &sdlEvent);

        switch (sdlEvent.type) {
            case SDL_EVENT_MOUSE_MOTION:
                mouseX = sdlEvent.motion.x;
                mouseY = sdlEvent.motion.y;
                if (state == GameState::Menu) {
                    for (int i = 0; i < MENU_COUNT; i++) {
                        if (UI::Contains(MenuButtonRect(i), mouseX, mouseY)) menuIndex = i;
                    }
                }
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
                    mouseX = sdlEvent.button.x;
                    mouseY = sdlEvent.button.y;
                    OnClick();
                }
                break;
            case SDL_EVENT_KEY_DOWN:
                if (!sdlEvent.key.repeat) {
                    OnKeyDown(sdlEvent.key.key);
                }
                break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Setup / Update
////////////////////////////////////////////////////////////////////////////////
void Game::Setup() {
    // The game starts in the main menu; the level is built when the player presses PLAY
    SetState(GameState::Menu);
}

void Game::Update() {
    Uint64 now = SDL_GetTicks();
    double deltaTime = (now - millisecsPreviousFrame) / 1000.0;
    millisecsPreviousFrame = now;
    if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;

    // Menus, pause and result screens do not simulate the world
    if (state != GameState::Playing) {
        return;
    }

    // Process the entities that are waiting to be created/deleted
    registry->Update();

    // Score: every enemy that disappeared since last frame was destroyed
    int enemiesNow = CountEnemies();
    if (enemiesNow < enemiesLeft) {
        score += (enemiesLeft - enemiesNow) * SCORE_PER_ENEMY;
    }
    enemiesLeft = enemiesNow;

    // Level complete?
    if (enemiesTotal > 0 && enemiesLeft == 0) {
        score += LEVEL_CLEAR_BONUS;
        UpdateHighScore();
        SetState(LevelExists(currentLevel + 1) ? GameState::LevelComplete : GameState::Victory);
        return;
    }

    // Is the player still alive?
    auto players = registry->GetSystem<KeyboardControlSystem>().GetSystemEntities();
    if (players.empty()) {
        lives--;
        if (lives <= 0) {
            UpdateHighScore();
            SetState(GameState::GameOver);
        } else {
            SetState(GameState::PlayerDown);
        }
        return;
    }

    glm::vec2 playerCenter(0.0f);
    {
        const auto& playerTransform = players[0].GetComponent<TransformComponent>();
        playerCenter = playerTransform.position + glm::vec2(16.0f, 16.0f);
    }

    // Invoke all the systems that need to update
    registry->GetSystem<KeyboardControlSystem>().Update();
    registry->GetSystem<MovementSystem>().Update(deltaTime);
    registry->GetSystem<PatrolSystem>().Update();
    registry->GetSystem<AnimationSystem>().Update();
    registry->GetSystem<CollisionSystem>().Update(eventBus);
    registry->GetSystem<EnemyAISystem>().Update(true, playerCenter);
    registry->GetSystem<ProjectileEmitSystem>().Update(registry);
    registry->GetSystem<CameraMovementSystem>().Update(camera);
    registry->GetSystem<ProjectileLifecycleSystem>().Update();
    registry->GetSystem<ScriptSystem>().Update(deltaTime, static_cast<int>(now));
}

////////////////////////////////////////////////////////////////////////////////
// Rendering
////////////////////////////////////////////////////////////////////////////////
void Game::RenderMenu() {
    // Vertical gradient background
    for (int y = 0; y < windowHeight; y += 4) {
        float t = y / static_cast<float>(windowHeight);
        SDL_Color c = { static_cast<Uint8>(8 + 20 * t), static_cast<Uint8>(22 + 50 * t), static_cast<Uint8>(48 + 80 * t), 255 };
        UI::FillRect(renderer, 0, static_cast<float>(y), static_cast<float>(windowWidth), 4, c);
    }

    // A helicopter flying across the top of the screen
    SDL_Texture* chopper = uiAssets->GetTexture("menu-chopper");
    if (chopper) {
        Uint64 t = SDL_GetTicks();
        int frame = static_cast<int>((t / 100) % 2);
        SDL_FRect src = { static_cast<float>(frame * 32), 32.0f, 32.0f, 32.0f };   // row 1 = flying right
        float x = std::fmod(static_cast<float>(t) * 0.15f, windowWidth + 300.0f) - 150.0f;
        float y = 20.0f + 10.0f * std::sin(static_cast<float>(t) * 0.004f);
        SDL_FRect dst = { x, y, 128.0f, 128.0f };
        SDL_RenderTexture(renderer, chopper, &src, &dst);
    }

    const float cx = windowWidth / 2.0f;
    UI::DrawTextShadow(renderer, titleFont, "NOVA 2D", cx, 140, YELLOW, UI::Center);
    UI::DrawTextShadow(renderer, menuFont, "HELICOPTER STRIKE", cx, 255, WHITE, UI::Center);

    for (int i = 0; i < MENU_COUNT; i++) {
        SDL_FRect r = MenuButtonRect(i);
        bool selected = (i == menuIndex);
        UI::FillRect(renderer, r.x, r.y, r.w, r.h, selected ? SDL_Color{ 255, 210, 60, 235 } : SDL_Color{ 255, 255, 255, 30 });
        UI::StrokeRect(renderer, r.x, r.y, r.w, r.h, selected ? WHITE : GREY);
        UI::DrawText(renderer, menuFont, MENU_ITEMS[i], cx, r.y + 7, selected ? DARK : WHITE, UI::Center);
    }

    UI::DrawTextShadow(renderer, hudFont, "BEST SCORE: " + std::to_string(highScore), cx, 590, GREEN, UI::Center);
    UI::DrawText(renderer, smallFont, "UP / DOWN + ENTER, or use the mouse        F11: fullscreen", cx, 685, GREY, UI::Center);
}

void Game::RenderControls() {
    for (int y = 0; y < windowHeight; y += 4) {
        float t = y / static_cast<float>(windowHeight);
        SDL_Color c = { static_cast<Uint8>(8 + 20 * t), static_cast<Uint8>(22 + 50 * t), static_cast<Uint8>(48 + 80 * t), 255 };
        UI::FillRect(renderer, 0, static_cast<float>(y), static_cast<float>(windowWidth), 4, c);
    }

    const float cx = windowWidth / 2.0f;
    UI::DrawTextShadow(renderer, titleFont, "CONTROLS", cx, 60, YELLOW, UI::Center);

    const float left = 300.0f;
    float y = 200.0f;
    auto line = [&](const std::string& key, const std::string& action) {
        UI::DrawText(renderer, hudFont, key, left, y, YELLOW);
        UI::DrawText(renderer, hudFont, action, left + 300.0f, y, WHITE);
        y += 42.0f;
    };
    line("ARROW KEYS / WASD", "Fly the helicopter");
    line("SPACE (hold)", "Fire in the direction you face");
    line("P or ESC", "Pause");
    line("F1", "Debug tools");
    line("F11", "Fullscreen");

    y += 20.0f;
    UI::DrawText(renderer, hudFont, "MISSION: destroy every enemy to clear the level.", cx, y, GREEN, UI::Center);
    UI::DrawText(renderer, hudFont, "Enemies shoot back when you get close. You have 3 lives.", cx, y + 36.0f, WHITE, UI::Center);
    UI::DrawText(renderer, smallFont, "Press ENTER or click to go back", cx, 670, GREY, UI::Center);
}

void Game::RenderWorld() {
    registry->GetSystem<RenderSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderTextSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderHealthBarSystem>().Update(renderer, assetStore, camera);
    if (isDebug && (state == GameState::Playing || state == GameState::Paused)) {
        registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
        registry->GetSystem<RenderGUISystem>().Update(renderer, registry, camera);
    }
}

void Game::RenderHUD() {
    const float w = static_cast<float>(windowWidth);

    // Top bar
    UI::FillRect(renderer, 0, 0, w, 36, SDL_Color{ 0, 0, 0, 150 });
    UI::DrawText(renderer, hudFont, "LEVEL " + std::to_string(currentLevel) + ": " + levelName, 14, 7, WHITE);
    UI::DrawText(renderer, hudFont, "SCORE " + std::to_string(score), w / 2.0f, 7, YELLOW, UI::Center);
    UI::DrawText(renderer, hudFont,
        "LIVES " + std::to_string(lives) + "    ENEMIES " + std::to_string(enemiesLeft) + "/" + std::to_string(enemiesTotal),
        w - 14, 7, WHITE, UI::Right);

    // Player health bar (bottom-left)
    int hp = 0;
    auto players = registry->GetSystem<KeyboardControlSystem>().GetSystemEntities();
    if (!players.empty() && players[0].HasComponent<HealthComponent>()) {
        hp = std::max(0, players[0].GetComponent<HealthComponent>().healthPercentage);
    }
    const float barX = 20.0f, barY = static_cast<float>(windowHeight) - 44.0f, barW = 260.0f, barH = 24.0f;
    SDL_Color hpColor = hp >= 60 ? GREEN : (hp >= 30 ? YELLOW : RED);
    UI::FillRect(renderer, barX - 4, barY - 4, barW + 8, barH + 8, SDL_Color{ 0, 0, 0, 160 });
    UI::FillRect(renderer, barX, barY, barW * (std::min(hp, 100) / 100.0f), barH, hpColor);
    UI::StrokeRect(renderer, barX, barY, barW, barH, WHITE);
    UI::DrawText(renderer, smallFont, "HP " + std::to_string(hp), barX + barW / 2.0f, barY + 3, DARK, UI::Center);

    // Controls hint during the first seconds of a level
    if (state == GameState::Playing && SDL_GetTicks() - levelStartTime < 8000) {
        UI::DrawTextShadow(renderer, hudFont, "ARROWS / WASD: fly     SPACE: fire     P: pause", w / 2.0f, 640, WHITE, UI::Center);
    }
}

void Game::RenderOverlay() {
    if (state == GameState::Playing) return;

    const float w = static_cast<float>(windowWidth);
    const float h = static_cast<float>(windowHeight);
    const float cx = w / 2.0f;

    UI::FillRect(renderer, 0, 0, w, h, SDL_Color{ 0, 0, 0, 150 });
    UI::FillRect(renderer, cx - 300, 190, 600, 300, SDL_Color{ 15, 20, 35, 240 });
    UI::StrokeRect(renderer, cx - 300, 190, 600, 300, YELLOW);

    switch (state) {
        case GameState::Paused:
            UI::DrawText(renderer, menuFont, "PAUSED", cx, 225, YELLOW, UI::Center);
            UI::DrawText(renderer, hudFont, "P or ESC  -  resume", cx, 320, WHITE, UI::Center);
            UI::DrawText(renderer, hudFont, "Q  -  quit to the main menu", cx, 360, WHITE, UI::Center);
            UI::DrawText(renderer, hudFont, "F1  -  debug tools", cx, 400, GREY, UI::Center);
            break;
        case GameState::PlayerDown:
            UI::DrawText(renderer, menuFont, "HELICOPTER DOWN!", cx, 225, RED, UI::Center);
            UI::DrawText(renderer, hudFont, "Lives left: " + std::to_string(lives), cx, 320, WHITE, UI::Center);
            UI::DrawText(renderer, hudFont, "Press ENTER to try again", cx, 410, YELLOW, UI::Center);
            break;
        case GameState::LevelComplete:
            UI::DrawText(renderer, menuFont, "LEVEL " + std::to_string(currentLevel) + " COMPLETE!", cx, 225, GREEN, UI::Center);
            UI::DrawText(renderer, hudFont, "Score: " + std::to_string(score), cx, 320, WHITE, UI::Center);
            UI::DrawText(renderer, hudFont, "Level bonus: +" + std::to_string(LEVEL_CLEAR_BONUS), cx, 360, GREY, UI::Center);
            UI::DrawText(renderer, hudFont, "Press ENTER for the next level", cx, 420, YELLOW, UI::Center);
            break;
        case GameState::GameOver:
            UI::DrawText(renderer, menuFont, "GAME OVER", cx, 225, RED, UI::Center);
            UI::DrawText(renderer, hudFont, "Final score: " + std::to_string(score), cx, 320, WHITE, UI::Center);
            UI::DrawText(renderer, hudFont, "Best score: " + std::to_string(highScore), cx, 360, GREEN, UI::Center);
            UI::DrawText(renderer, hudFont, "Press ENTER for the main menu", cx, 420, YELLOW, UI::Center);
            break;
        case GameState::Victory:
            UI::DrawText(renderer, menuFont, "VICTORY!", cx, 225, YELLOW, UI::Center);
            UI::DrawText(renderer, hudFont, "You cleared all " + std::to_string(currentLevel) + " levels", cx, 320, WHITE, UI::Center);
            UI::DrawText(renderer, hudFont, "Final score: " + std::to_string(score) + "   Best: " + std::to_string(highScore), cx, 360, GREEN, UI::Center);
            UI::DrawText(renderer, hudFont, "Press ENTER for the main menu", cx, 420, YELLOW, UI::Center);
            break;
        default:
            break;
    }
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    switch (state) {
        case GameState::Menu:
            RenderMenu();
            break;
        case GameState::Controls:
            RenderControls();
            break;
        default:
            RenderWorld();
            RenderHUD();
            RenderOverlay();
            break;
    }

    SDL_RenderPresent(renderer);
}

void Game::Run() {
    if (!isRunning) return;   // Initialize() failed

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
