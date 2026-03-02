#include <SDL3/SDL.h>
#include<stdio.h>
#include<math.h>
#include"draw.h"

void draw(SDL_Renderer* p_renderer, SDL_Texture* p_draw_texture, SDL_Texture* p_target_texture,SDL_FRect* p_src_rect, SDL_FRect* p_dst_rect) {
    if(p_renderer == NULL) { return; }
    if(SDL_GetRenderTarget(p_renderer) != NULL || p_target_texture != NULL) {
        SDL_SetRenderTarget(p_renderer, p_target_texture);
    }
    if(p_draw_texture != NULL) {
        SDL_RenderTexture(p_renderer, p_draw_texture, p_src_rect, p_dst_rect);
    } else {
        SDL_SetRenderDrawColor(p_renderer, 0x44, 0x44, 0x44, 255);
        SDL_RenderClear(p_renderer);
    }
    if(p_target_texture == NULL) {
        SDL_RenderPresent(p_renderer);
    }
}

SDL_FRect get_rect(int x1, int x2, int y1, int y2) {
    SDL_FRect rect;
    rect.x = x1 < x2 ? x1 : x2;
    rect.y = y1 < y2 ? y1 : y2;
    rect.w = x1 < x2 ? x2 - x1 : x1 - x2;
    rect.h = y1 < y2 ? y2 - y1 : y1 - y2;
    return rect;
}

void draw_rect(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const SDL_FRect* p_rect, const RGBA color) {
    if(p_renderer == NULL || p_rect == NULL) { return; }
    SDL_SetRenderTarget(p_renderer, p_texture);
    SDL_SetRenderDrawColor(p_renderer, color.red, color.green, color.blue, color.alpha);
    SDL_RenderRect(p_renderer, p_rect);
    draw(p_renderer, p_texture, NULL, NULL, NULL);
}

void draw_line(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const float x1, const float x2, const float y1, const float y2, const RGBA color) {
    if(p_renderer == NULL || p_texture == NULL) { return; }
    SDL_SetRenderTarget(p_renderer, p_texture);
    SDL_SetRenderDrawColor(p_renderer, color.red, color.green, color.blue, color.alpha);
    SDL_RenderLine(p_renderer, x1, y1, x2, y2);
    draw(p_renderer, p_texture, NULL, NULL, NULL);
}

void draw_circle(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const float x, const float y, const float radius, const RGBA color) {
    if(p_renderer == NULL || p_texture == NULL) { return; }
    SDL_SetRenderTarget(p_renderer, p_texture);
    SDL_SetRenderDrawColor(p_renderer, color.red, color.green, color.blue, color.alpha);
    int a,b = 0;
    SDL_RenderPoint(p_renderer, x, y);
    while(b < radius) {
        a = sqrt(radius*radius - b*b);
        for(int i = 0; i < a; ++i) {
            SDL_RenderPoint(p_renderer, x+i, y+b);
            SDL_RenderPoint(p_renderer, x-i, y+b);
            SDL_RenderPoint(p_renderer, x+i, y-b);
            SDL_RenderPoint(p_renderer, x-i, y-b);
        }
        ++b;
    }
    draw(p_renderer, p_texture, NULL, NULL, NULL);
}

void draw_select_box(SDL_Renderer* p_renderer, SDL_Texture* p_texture, float x1, float x2, float y1, float y2) {
    if(p_renderer == NULL || p_texture == NULL) { return; }
    SDL_SetRenderTarget(p_renderer, p_texture);
    SDL_SetRenderDrawColor(p_renderer, 0xFF, 0xFF, 0xFF, 255);
    const float max_x = x1 > x2 ? x1 : x2;
    const float min_x = x1 < x2 ? x1 : x2;
    const float max_y = y1 > y2 ? y1 : y2;
    const float min_y = y1 < y2 ? y1 : y2;
    x1 = min_x;
    y1 = min_y;
    x2 = x1 + 10;
    y2 = y1 + 10;
    while(x2 < max_x) {
        SDL_RenderLine(p_renderer, x1, min_y, x2, min_y);
        x1 = x2 + 10;
        x2 = x1 + 10;
    }
    while(y2 < max_y) {
        SDL_RenderLine(p_renderer, max_x, y1, max_x, y2);
        y1 = y2 + 10;
        y2 = y1 + 10;
    }
    x1 = max_x;
    x2 = x1 - 10;
    while(x2 > min_x) {
        SDL_RenderLine(p_renderer, x1, max_y, x2, max_y);
        x1 = x2 - 10;
        x2 = x1 - 10;
    }
    y1 = max_y;
    y2 = y1 - 10;
    while(y2 > min_y) {
        SDL_RenderLine(p_renderer, min_x, y1, min_x, y2);
        y1 = y2 - 10;
        y2 = y1 - 10;
    }
    draw(p_renderer, p_texture, NULL, NULL, NULL);
}

SDL_Texture* copy_texture(SDL_Renderer* p_renderer, SDL_Texture* p_texture, SDL_FRect* p_src_rect) { 
    if(p_renderer == NULL || p_texture == NULL) { return NULL; }
    SDL_Texture* p_temp_texture;
    if(p_src_rect == NULL) {
        p_temp_texture = SDL_CreateTexture(p_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, p_texture->w, p_texture->h);
    } else {
        p_temp_texture = SDL_CreateTexture(p_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, p_src_rect->w, p_src_rect->h);
    }
    if(p_temp_texture == NULL) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return NULL;
    }
    draw(p_renderer, p_texture, p_temp_texture, p_src_rect, NULL);
    return p_temp_texture;
}
