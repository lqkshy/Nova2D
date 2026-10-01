#ifndef UI_H
#define UI_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <string>

// Tiny helpers to draw menus, text and the HUD with plain SDL.
namespace UI {
    enum Align { Left = 0, Center = 1, Right = 2 };

    inline void FillRect(SDL_Renderer* renderer, float x, float y, float w, float h, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_FRect rect = { x, y, w, h };
        SDL_RenderFillRect(renderer, &rect);
    }

    inline void StrokeRect(SDL_Renderer* renderer, float x, float y, float w, float h, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_FRect rect = { x, y, w, h };
        SDL_RenderRect(renderer, &rect);
    }

    inline bool Contains(const SDL_FRect& rect, float px, float py) {
        return px >= rect.x && px <= rect.x + rect.w && py >= rect.y && py <= rect.y + rect.h;
    }

    inline void DrawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, float x, float y, SDL_Color color, Align align = Left) {
        if (!font || text.empty()) return;
        SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
        if (!surface) return;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
        if (!texture) return;

        float w = 0, h = 0;
        SDL_GetTextureSize(texture, &w, &h);
        float drawX = x;
        if (align == Center) drawX = x - w / 2;
        if (align == Right)  drawX = x - w;

        SDL_FRect dst = { drawX, y, w, h };
        SDL_RenderTexture(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }

    // Text with a dark drop shadow (easier to read on top of the map)
    inline void DrawTextShadow(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, float x, float y, SDL_Color color, Align align = Left) {
        DrawText(renderer, font, text, x + 2, y + 2, SDL_Color{ 0, 0, 0, 200 }, align);
        DrawText(renderer, font, text, x, y, color, align);
    }
}

#endif
