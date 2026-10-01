#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"
#include <SDL3/SDL.h>
#include <algorithm>

class RenderSystem: public System {
    public:
        RenderSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<SpriteComponent>();
        }

        void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, SDL_Rect& camera) {
            // Create a vector with both Sprite and Transform component of all entities
            struct RenderableEntity {
                TransformComponent transformComponent;
                SpriteComponent spriteComponent;
            };
            std::vector<RenderableEntity> renderableEntities; 
            for (auto entity: GetSystemEntities()) {
                RenderableEntity renderableEntity;
                renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();
                renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();

                // Check if the entity sprite is outside the camera view
                bool isOutsideCameraView = (
                    renderableEntity.transformComponent.position.x + (renderableEntity.transformComponent.scale.x * renderableEntity.spriteComponent.width) < camera.x ||
                    renderableEntity.transformComponent.position.x > camera.x + camera.w ||
                    renderableEntity.transformComponent.position.y + (renderableEntity.transformComponent.scale.y * renderableEntity.spriteComponent.height) < camera.y ||
                    renderableEntity.transformComponent.position.y > camera.y + camera.h
                );

                // Cull sprites that are outside the camera viww (and are not fixed)
                if (isOutsideCameraView && !renderableEntity.spriteComponent.isFixed) {
                    continue;
                }

                renderableEntities.emplace_back(renderableEntity);
            }

            // Sort the vector by the z-index value
            std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
                return a.spriteComponent.zIndex < b.spriteComponent.zIndex;
            });

            // Loop all entities that the system is interested in
            for (auto entity : renderableEntities) {
                const auto transform = entity.transformComponent;
                const auto sprite = entity.spriteComponent;

                SDL_FRect srcRect = sprite.srcRect;
                SDL_FRect dstRect = {
                    transform.position.x - (sprite.isFixed ? 0 : (float)camera.x),
                    transform.position.y - (sprite.isFixed ? 0 : (float)camera.y),
                    sprite.width * transform.scale.x,
                    sprite.height * transform.scale.y
                };

                SDL_RenderTextureRotated(
                    renderer,
                    assetStore->GetTexture(sprite.assetId),
                    &srcRect,
                    &dstRect,
                    transform.rotation,
                    NULL,
                    sprite.flip
                );
            }
        }
};

#endif
