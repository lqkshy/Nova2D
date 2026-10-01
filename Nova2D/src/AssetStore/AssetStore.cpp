#include "./AssetStore.h"
#include "../Logger/Logger.h"
#include <SDL3/SDL_image.h>

AssetStore::AssetStore() {
    Logger::Log("AssetStore constructor called!");
}

AssetStore::~AssetStore() {
    ClearAssets();
    Logger::Log("AssetStore destructor called!");
}

void AssetStore::ClearAssets() {
    for (auto texture: textures) {
        SDL_DestroyTexture(texture.second);
    }
    textures.clear();

    for (auto font: fonts) {
        TTF_CloseFont(font.second);
    }
    fonts.clear();
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath) {
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        Logger::Err("Could not load texture file: " + filePath);
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (!texture) {
        Logger::Err("Could not create texture for: " + filePath);
        return;
    }

    // Crisp pixel art (no blurry scaling)
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    // Replace an existing texture with the same id instead of leaking it
    auto existing = textures.find(assetId);
    if (existing != textures.end()) {
        SDL_DestroyTexture(existing->second);
        existing->second = texture;
    } else {
        textures.emplace(assetId, texture);
    }

    Logger::Log("Texture added to the AssetStore with id " + assetId);
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId) {
    auto texture = textures.find(assetId);
    return texture != textures.end() ? texture->second : nullptr;
}

void AssetStore::AddFont(const std::string& assetId, const std::string& filePath, int fontSize) {
    TTF_Font* font = TTF_OpenFont(filePath.c_str(), static_cast<float>(fontSize));
    if (!font) {
        Logger::Err("Could not load font file: " + filePath);
        return;
    }
    auto existing = fonts.find(assetId);
    if (existing != fonts.end()) {
        TTF_CloseFont(existing->second);
        existing->second = font;
    } else {
        fonts.emplace(assetId, font);
    }
}

TTF_Font* AssetStore::GetFont(const std::string& assetId) {
    auto font = fonts.find(assetId);
    return font != fonts.end() ? font->second : nullptr;
}
