#include <SDL3/SDL_render.h>
#include <assert.h>
#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<SDL3/SDL.h>
#include<SDL3/SDL_main.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define ACTION_STACK_MAX_SIZE 64

typedef struct Action{
    SDL_Texture* p_data;
    struct Action* p_prev;
} Action; 

typedef struct Window {
    SDL_Window* p_window;
    SDL_Renderer* p_screen_renderer;
    SDL_Texture* p_texture;
    SDL_Texture* p_clipboard_texture;
    SDL_FRect clipboard_rect;
    Action* p_action_head;
    int stack_size;
} Window;

void close_sdl(Window* p_win) {
    if(p_win == NULL) {
        SDL_Quit();
        return;
    }
    while(p_win->p_action_head != NULL) {
        Action* p_temp = p_win->p_action_head->p_prev;
        SDL_DestroyTexture(p_win->p_action_head->p_data);
        free(p_win->p_action_head);
        p_win->p_action_head = p_temp;
    }
    if(p_win->p_clipboard_texture != NULL) {
        SDL_DestroyTexture(p_win->p_clipboard_texture);
    }
    if(p_win->p_texture != NULL) {
        SDL_DestroyTexture(p_win->p_texture);
    }
    if(p_win->p_screen_renderer != NULL) {
        SDL_DestroyRenderer(p_win->p_screen_renderer);
    }
    if(p_win->p_window != NULL) {
        SDL_DestroyWindow(p_win->p_window);
    }
    SDL_Quit();
    free(p_win);
}

Window* create_window() {
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Couldn't init: %s\n", SDL_GetError());
        return NULL;
    }
    Window* p_win = malloc(sizeof(Window));
    SDL_CreateWindowAndRenderer("Draw", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &p_win->p_window, &p_win->p_screen_renderer);
    if(p_win->p_window == NULL) {
        printf("Failed to create window: %s\n", SDL_GetError());
        close_sdl(p_win);
        return NULL;
    }
    if(p_win->p_screen_renderer == NULL) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        close_sdl(p_win);
        return NULL;
    }
    return p_win;
}

/* Draws p_draw_texture to p_target_texture at p_dst_rect, if target texture is NULL it will draw to the window instead.
 * If draw texture is NULL, will color target texture gray.
 * Should maybe split this into draw_texture and draw_window
*/
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

// Returns a rectangle with the two points as opposite corners.
SDL_FRect get_rect(int x1, int x2, int y1, int y2) {
    SDL_FRect rect;
    rect.x = x1 < x2 ? x1 : x2;
    rect.y = y1 < y2 ? y1 : y2;
    rect.w = x1 < x2 ? x2 - x1 : x1 - x2;
    rect.h = y1 < y2 ? y2 - y1 : y1 - y2;
    return rect;
}

void draw_rect(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const SDL_FRect* p_rect) {
    if(p_renderer == NULL || p_rect == NULL) { return; }
    SDL_SetRenderTarget(p_renderer, p_texture);
    SDL_SetRenderDrawColor(p_renderer, 0xFF, 0xFF, 0xFF, 255);
    SDL_RenderRect(p_renderer, p_rect);
    draw(p_renderer, p_texture, NULL, NULL, NULL);
}

void draw_line(SDL_Renderer* p_renderer, SDL_Texture* p_texture, const float x1, const float x2, const float y1, const float y2) {
    if(p_renderer == NULL || p_texture == NULL) { return; }
    SDL_SetRenderTarget(p_renderer, p_texture);
    SDL_SetRenderDrawColor(p_renderer, 0xFF, 0xFF, 0xFF, 255);
    SDL_RenderLine(p_renderer, x1, y1, x2, y2);
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

/* Could be useful
void draw_point(SDL_Renderer* p_renderer, SDL_Texture* p_texture, float x, float y) {
    SDL_SetRenderTarget(p_renderer, p_texture);
    SDL_SetRenderDrawColor(p_renderer, 0xFF, 0xFF, 0xFF, 255);
    SDL_RenderPoint(p_renderer, x, y);
    SDL_SetRenderTarget(p_renderer, NULL);
    SDL_RenderTexture(p_renderer, p_texture, NULL, NULL);
    SDL_RenderPresent(p_renderer);
}*/

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

void pop_action(Action** p_p_head, int* p_stack_size) {
    Action* p_temp = (*p_p_head)->p_prev;
    SDL_DestroyTexture((*p_p_head)->p_data);
    free(*p_p_head);
    *p_p_head = p_temp;
    --*p_stack_size;
}

void add_action(Window* p_win) {
    Action* p_new_action = malloc(sizeof(Action));
    p_new_action->p_data = copy_texture(p_win->p_screen_renderer, p_win->p_texture,NULL);
    p_new_action->p_prev = p_win->p_action_head;
    p_win->p_action_head = p_new_action;
    if(p_win->stack_size < ACTION_STACK_MAX_SIZE) {
        ++p_win->stack_size;
        return;
    }
    Action* p_temp = p_win->p_action_head;
    Action* p_delete = NULL;
    while(p_temp->p_prev != NULL) {
        if(p_temp->p_prev->p_prev == NULL) { // Second to last one!
            p_delete = p_temp->p_prev; // Last one!
            break;
        }
        p_temp = p_temp->p_prev;
    }
    if(p_delete == NULL) {
        return;
    }
    SDL_DestroyTexture(p_delete->p_data);
    free(p_delete);
    p_temp->p_prev = NULL;
}

int main() {
    Window* p_win = create_window();
    if(p_win == NULL) {
        printf("Window empty, see above errors");
        return 1;
    }
    p_win->p_action_head = NULL;
    p_win->p_clipboard_texture = NULL;
    p_win->stack_size = 0;
    bool b_quit, b_drawing , b_resizing, b_update, b_selecting, b_draw_rect = false;
    b_quit = b_drawing = b_resizing = b_selecting = b_draw_rect;
    b_update = true;
    float start_x = 0;
    float start_y = 0;
    // Creating the texture we will draw everything to and presenting
    int texture_width, texture_height;
    SDL_GetWindowSize(p_win->p_window, &texture_width, &texture_height);
    p_win->p_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
    if(p_win->p_texture == NULL) {
        printf("Failed to create main texture: SDL_Error %s\n", SDL_GetError());
        close_sdl(p_win);
    }
    draw(p_win->p_screen_renderer, NULL, p_win->p_texture, NULL, NULL);
    draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
    add_action(p_win);
    while (!b_quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    b_quit = true;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    ; // Label can't be followed by a declaration before C23
                    SDL_FRect src_rect = {0,0,texture_width,texture_height};
                    SDL_GetWindowSize(p_win->p_window, &texture_width, &texture_height);
                    SDL_Texture* new_size_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
                    if(new_size_texture == NULL) {
                        printf("Failed to create resize texture: SDL_Error %s\n", SDL_GetError());
                        close_sdl(p_win);
                    }
                    SDL_SetRenderTarget(p_win->p_screen_renderer, new_size_texture);
                    SDL_SetRenderDrawColor(p_win->p_screen_renderer, 0x44, 0x44, 0x44, 255);
                    SDL_RenderClear(p_win->p_screen_renderer);
                    SDL_RenderTexture(p_win->p_screen_renderer, p_win->p_texture, NULL, &src_rect);
                    SDL_DestroyTexture(p_win->p_texture);
                    p_win->p_texture = new_size_texture;
                    draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
                    if(b_resizing) {
                        break;
                    }
                    add_action(p_win);
                    b_resizing = true;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    b_resizing = false;
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        b_drawing = true;
                        start_x = e.button.x;
                        start_y = e.button.y;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    b_resizing = false;
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        b_drawing = false;
                        if(b_draw_rect) {
                            SDL_FRect rect = get_rect(start_x, e.button.x, start_y, e.button.y);
                            draw_rect(p_win->p_screen_renderer, p_win->p_texture, &rect);
                        } else if(b_selecting) {
                            SDL_FRect rect = get_rect(start_x, e.button.x, start_y, e.button.y);
                            SDL_DestroyTexture(p_win->p_clipboard_texture);
                            p_win->p_clipboard_texture = copy_texture(p_win->p_screen_renderer, p_win->p_texture, &rect);
                            p_win->clipboard_rect = rect;
                        }
                        add_action(p_win);
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    b_resizing = false;
                    if (!b_drawing) {
                        break;
                    }
                    if(b_draw_rect) {
                        SDL_Texture* temp_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
                        if(temp_texture == NULL) {
                            printf("Failed to create box indicator texture: SDL_Error %s\n", SDL_GetError());
                            close_sdl(p_win);
                        }
                        draw(p_win->p_screen_renderer, p_win->p_texture, temp_texture, NULL, NULL);
                        SDL_FRect rect = get_rect(start_x, e.motion.x, start_y, e.motion.y);
                        draw_rect(p_win->p_screen_renderer, temp_texture, &rect);
                        SDL_DestroyTexture(temp_texture);
                        break;
                    } else if(b_selecting) {
                        SDL_Texture* temp_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
                        if(temp_texture == NULL) {
                            printf("Failed to create select box indicator texture: SDL_Error %s\n", SDL_GetError());
                            close_sdl(p_win);
                        }
                        draw(p_win->p_screen_renderer, p_win->p_texture, temp_texture, NULL, NULL);
                        draw_select_box(p_win->p_screen_renderer, temp_texture, start_x, e.motion.x, start_y, e.motion.y);
                        SDL_DestroyTexture(temp_texture);
                        break;
                    } else {
                        draw_line(p_win->p_screen_renderer, p_win->p_texture, start_x, e.motion.x, start_y, e.motion.y);
                    }
                    if(b_update) {
                        start_x = e.motion.x;
                        start_y = e.motion.y;
                    }
                    break;
                case SDL_EVENT_KEY_DOWN:
                    b_resizing = false;
                    if(e.key.key == SDLK_SPACE) {
                        draw(p_win->p_screen_renderer, NULL, p_win->p_texture, NULL, NULL);
                        draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
                        add_action(p_win);
                    } else if (e.key.key == SDLK_C) {
                        b_update = !b_update;
                        b_selecting = false;
                        b_draw_rect = false;
                    } else if (e.key.key == SDLK_B) {
                        b_draw_rect = !b_draw_rect;
                        b_update = !b_draw_rect;
                        b_selecting = false;
                    } else if (e.key.key == SDLK_S) {
                        b_selecting = !b_selecting;
                        b_update = !b_selecting;
                        b_draw_rect = false;
                    } else if(e.key.key == SDLK_X) {
                        if(p_win->p_clipboard_texture != NULL) {
                            float center_x, center_y;
                            SDL_GetMouseState(&center_x, &center_y);
                            center_x -= (p_win->clipboard_rect.w/2);
                            center_y -= (p_win->clipboard_rect.h/2);
                            SDL_FRect dst_rect = {center_x, center_y, p_win->clipboard_rect.w, p_win->clipboard_rect.h};
                            draw(p_win->p_screen_renderer, p_win->p_clipboard_texture, p_win->p_texture, NULL, &dst_rect);
                            draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
                            add_action(p_win);
                            b_selecting = false;
                        }
                    } else if (e.key.key == SDLK_Z) {
                        if(p_win->p_action_head->p_prev != NULL) {
                            pop_action(&p_win->p_action_head, &p_win->stack_size);
                            SDL_FRect src_rect = {0,0,p_win->p_action_head->p_data->w, p_win->p_action_head->p_data->h};
                            draw(p_win->p_screen_renderer, NULL, p_win->p_texture, NULL, NULL);
                            draw(p_win->p_screen_renderer, p_win->p_action_head->p_data, p_win->p_texture, NULL, &src_rect);
                            draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
                        }
                    }
                    break;
            }
        }
    }
    close_sdl(p_win);
    return 0;
}
