#include<SDL3/SDL.h>

/* Draws p_draw_texture to p_target_texture at p_dst_rect, if target texture is NULL it will draw to the window instead.
 * If draw texture is NULL, will color target texture gray.
 * Should maybe split this into draw_texture and draw_window
 */
void draw(SDL_Renderer* p_renderer, SDL_Texture* p_draw_texture, SDL_Texture* p_target_texture,SDL_FRect* p_src_rect, SDL_FRect* p_dst_rect);
// Returns a rectangle with the two points as opposite corners.
SDL_FRect get_rect(int x1, int x2, int y1, int y2);
// Draws a rectangle to the texture.
void draw_rect(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const SDL_FRect* p_rect);
// Draws a line to the texture from x1,y1 to x2,y2
void draw_line(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const float x1, const float x2, const float y1, const float y2);
// Draws a rectangle with dotted lines, doesn't use SDL_RenderRect, it's a grouping of lines.
void draw_select_box(SDL_Renderer* p_renderer, SDL_Texture* p_texture, float x1, float x2, float y1, float y2);
// Returns a new texture that has the same content rendered as p_texture.
SDL_Texture* copy_texture(SDL_Renderer* p_renderer, SDL_Texture* p_texture, SDL_FRect* p_src_rect);
