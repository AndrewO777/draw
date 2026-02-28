#include<SDL3/SDL.h>

void draw(SDL_Renderer* p_renderer, SDL_Texture* p_draw_texture, SDL_Texture* p_target_texture,SDL_FRect* p_src_rect, SDL_FRect* p_dst_rect);
SDL_FRect get_rect(int x1, int x2, int y1, int y2);
void draw_rect(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const SDL_FRect* p_rect);
void draw_line(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const float x1, const float x2, const float y1, const float y2);
void draw_select_box(SDL_Renderer* p_renderer, SDL_Texture* p_texture, float x1, float x2, float y1, float y2);
SDL_Texture* copy_texture(SDL_Renderer* p_renderer, SDL_Texture* p_texture, SDL_FRect* p_src_rect);
