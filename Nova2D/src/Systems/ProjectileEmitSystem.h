#ifndef PROJECTILEEMITSYSTEM_H
#define PROJECTILEEMITSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/ProjectileComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/KeyboardControlledComponent.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

constexpr int PLAYER_FIRE_COOLDOWN_MS = 250;

class ProjectileEmitSystem: public System {
    private:
        static void SpawnProjectile(std::unique_ptr<Registry>& registry, const glm::vec2& position, const glm::vec2& velocity, const ProjectileEmitterComponent& emitter) {
            Entity projectile = registry->CreateEntity();
            projectile.Group("projectiles");
            projectile.AddComponent<TransformComponent>(position - glm::vec2(2.0f, 2.0f), glm::vec2(1.0, 1.0), 0.0);
            projectile.AddComponent<RigidBodyComponent>(velocity);
            projectile.AddComponent<SpriteComponent>("bullet-texture", 4, 4, 4);
            projectile.AddComponent<BoxColliderComponent>(4, 4);
            projectile.AddComponent<ProjectileComponent>(emitter.isFriendly, emitter.hitPercentDamage, emitter.projectileDuration);
        }

    public:
        ProjectileEmitSystem() {
            RequireComponent<ProjectileEmitterComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(std::unique_ptr<Registry>& registry) {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            const bool firePressed = keys[SDL_SCANCODE_SPACE];
            const int now = static_cast<int>(SDL_GetTicks());

            for (auto entity: GetSystemEntities()) {
                auto& emitter = entity.GetComponent<ProjectileEmitterComponent>();

                // Start the bullet in the middle of the entity (read BEFORE creating the projectile,
                // because creating it can move the component pools in memory)
                const auto& transform = entity.GetComponent<TransformComponent>();
                glm::vec2 origin = transform.position;
                if (entity.HasComponent<SpriteComponent>()) {
                    const auto& sprite = entity.GetComponent<SpriteComponent>();
                    origin.x += (transform.scale.x * sprite.width / 2);
                    origin.y += (transform.scale.y * sprite.height / 2);
                }

                // The player fires while SPACE is held, in the direction it is facing
                if (entity.HasTag("player")) {
                    if (!firePressed || now - emitter.lastEmissionTime < PLAYER_FIRE_COOLDOWN_MS) {
                        continue;
                    }
                    glm::vec2 direction(0.0f, -1.0f);
                    if (entity.HasComponent<KeyboardControlledComponent>()) {
                        direction = entity.GetComponent<KeyboardControlledComponent>().facing;
                    }
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0.0f) direction /= length;
                    float speed = std::max(std::abs(emitter.projectileVelocity.x), std::abs(emitter.projectileVelocity.y));

                    SpawnProjectile(registry, origin, direction * speed, emitter);
                    emitter.lastEmissionTime = now;
                    continue;
                }

                // Everybody else (enemies) fires automatically every repeatFrequency milliseconds
                if (emitter.repeatFrequency == 0) {
                    continue;
                }
                if (now - emitter.lastEmissionTime > emitter.repeatFrequency) {
                    SpawnProjectile(registry, origin, emitter.projectileVelocity, emitter);
                    emitter.lastEmissionTime = now;
                }
            }
        }
};

#endif
