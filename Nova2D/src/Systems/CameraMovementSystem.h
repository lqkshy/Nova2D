#ifndef CAMERAMOVEMENTSYSTEM_H
#define CAMERAMOVEMENTSYSTEM_H

#include "../Game/Game.h"
#include "../ECS/ECS.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include <SDL3/SDL.h>
#include <algorithm>

class CameraMovementSystem: public System {
    public:
        CameraMovementSystem() {
            RequireComponent<CameraFollowComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(SDL_Rect& camera) {
            for (auto entity: GetSystemEntities()) {
                const auto& transform = entity.GetComponent<TransformComponent>();

                // Center the camera on the entity, then keep it inside the map
                camera.x = static_cast<int>(transform.position.x) + 16 - camera.w / 2;
                camera.y = static_cast<int>(transform.position.y) + 16 - camera.h / 2;

                const int maxX = std::max(0, Game::mapWidth - camera.w);
                const int maxY = std::max(0, Game::mapHeight - camera.h);
                camera.x = std::clamp(camera.x, 0, maxX);
                camera.y = std::clamp(camera.y, 0, maxY);
            }
        }
};

#endif
