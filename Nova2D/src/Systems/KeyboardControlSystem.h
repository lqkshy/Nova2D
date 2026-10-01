#ifndef KEYBOARDCONTROLSYSTEM_H
#define KEYBOARDCONTROLSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/KeyboardControlledComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include <SDL3/SDL.h>
#include <cmath>

// Moves the player while the keys are HELD and stops it when they are released.
// (The old version only reacted to a key press, so the helicopter never stopped.)
class KeyboardControlSystem: public System {
    public:
        KeyboardControlSystem() {
            RequireComponent<KeyboardControlledComponent>();
            RequireComponent<SpriteComponent>();
            RequireComponent<RigidBodyComponent>();
        }

        void Update() {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            const bool up    = keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W];
            const bool right = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
            const bool down  = keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S];
            const bool left  = keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A];

            for (auto entity: GetSystemEntities()) {
                auto& control = entity.GetComponent<KeyboardControlledComponent>();
                auto& sprite = entity.GetComponent<SpriteComponent>();
                auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

                glm::vec2 velocity(0.0f);
                glm::vec2 direction(0.0f);
                if (up)    { velocity += control.upVelocity;    direction.y -= 1.0f; }
                if (right) { velocity += control.rightVelocity; direction.x += 1.0f; }
                if (down)  { velocity += control.downVelocity;  direction.y += 1.0f; }
                if (left)  { velocity += control.leftVelocity;  direction.x -= 1.0f; }

                // Same speed in diagonals
                if (velocity.x != 0.0f && velocity.y != 0.0f) {
                    velocity *= 0.70710678f;
                }
                rigidbody.velocity = velocity;

                if (direction.x != 0.0f || direction.y != 0.0f) {
                    control.facing = direction;

                    // Sprite sheet rows: 0 = up, 1 = right, 2 = down, 3 = left
                    int row = 0;
                    if (direction.x > 0.0f) row = 1;
                    else if (direction.x < 0.0f) row = 3;
                    else if (direction.y > 0.0f) row = 2;
                    sprite.srcRect.y = static_cast<float>(sprite.height * row);
                }
            }
        }
};

#endif
