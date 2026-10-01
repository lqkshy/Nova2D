#ifndef ENEMYAISYSTEM_H
#define ENEMYAISYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <cmath>

constexpr float ENEMY_FIRE_RANGE = 420.0f;   // enemies only shoot when the player is this close

// Makes enemies aim at the player and only shoot when the player is in range.
class EnemyAISystem: public System {
    public:
        EnemyAISystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<ProjectileEmitterComponent>();
        }

        void Update(bool playerAlive, const glm::vec2& playerCenter) {
            const int now = static_cast<int>(SDL_GetTicks());

            for (auto entity: GetSystemEntities()) {
                if (entity.HasTag("player")) {
                    continue;
                }

                auto& emitter = entity.GetComponent<ProjectileEmitterComponent>();
                const auto& transform = entity.GetComponent<TransformComponent>();

                glm::vec2 center = transform.position;
                if (entity.HasComponent<SpriteComponent>()) {
                    const auto& sprite = entity.GetComponent<SpriteComponent>();
                    center.x += transform.scale.x * sprite.width / 2;
                    center.y += transform.scale.y * sprite.height / 2;
                }

                glm::vec2 toPlayer = playerCenter - center;
                float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

                // Stationary tanks turn to face the player
                if (playerAlive && entity.HasComponent<SpriteComponent>() && entity.HasComponent<RigidBodyComponent>()) {
                    if (entity.GetComponent<RigidBodyComponent>().velocity.x == 0.0f) {
                        entity.GetComponent<SpriteComponent>().flip = (toPlayer.x < 0.0f) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                    }
                }

                // Out of range (or the player is dead): hold fire and restart the reload timer
                if (!playerAlive || distance > ENEMY_FIRE_RANGE || distance < 1.0f) {
                    emitter.lastEmissionTime = now;
                    continue;
                }

                // Aim: keep the bullet speed from the level file, only change the direction
                float speed = std::sqrt(emitter.projectileVelocity.x * emitter.projectileVelocity.x + emitter.projectileVelocity.y * emitter.projectileVelocity.y);
                if (speed < 1.0f) speed = 150.0f;
                emitter.projectileVelocity = (toPlayer / distance) * speed;
            }
        }
};

#endif
