#ifndef RENDERTEXTSYSTEM_H
#define RENDERTEXTSYSTEM_H

#include "../AssetStore/AssetStore.h"
#include "../ECS/ECS.h"
#include "../Components/TextLabelComponent.h"
#include <SDL3/SDL.h>

class RenderTextSystem: public System {
    public:
        RenderTextSystem() {
            RequireComponent<TextLabelComponent>();
        }

        void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const SDL_Rect& camera) {
            for (auto entity: GetSystemEntities()) {
                const auto textlabel = entity.GetComponent<TextLabelComponent>();

                SDL_Surface* surface = TTF_RenderText_Blended(
                    assetStore->GetFont(textlabel.assetId),
                    textlabel.text.c_str(),
                    0,
                    textlabel.color
                );
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_DestroySurface(surface);

                float labelWidth = 0, labelHeight = 0;
                SDL_GetTextureSize(texture, &labelWidth, &labelHeight);

                SDL_FRect dstRect = {
                    textlabel.position.x - (textlabel.isFixed ? 0 : (float)camera.x),
                    textlabel.position.y - (textlabel.isFixed ? 0 : (float)camera.y),
                    labelWidth,
                    labelHeight
                };

                SDL_RenderTexture(renderer, texture, NULL, &dstRect);
                SDL_DestroyTexture(texture);
            }
        }
};

#endif
