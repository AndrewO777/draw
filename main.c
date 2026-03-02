#include <math.h>
#include<stdbool.h>
#include<stdio.h>
#include"window.h"
#include"draw.h"

int main() {
    Window* p_win = create_window();
    if(p_win == NULL) {
        printf("Window empty, see above errors");
        return 1;
    }
    p_win->stack_size = 0;
    p_win->mode = 0;

    bool b_quit, b_drawing, b_picking_color, b_resizing = false;
    b_quit = b_drawing = b_picking_color = b_resizing;
    float start_x = 0;
    float start_y = 0;
    float picker_x = 0;
    float picker_y = 0;
    RGBA color = {0xFF,0xFF,0xFF,255};
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
                        if(b_picking_color) {
                            if(e.button.x <= picker_x+255 && 
                                    e.button.x >= picker_x &&
                                    e.button.y <= picker_y+255 &&
                                    e.button.y >= picker_y) {
                                RGBA new_color = {e.button.x-picker_x, e.button.y-picker_y, 255-fabsf((e.button.x-picker_x)-(e.button.y-picker_y)), 255};
                                color = new_color;
                            }
                            draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
                            break;
                        }
                        if(p_win->mode == DRAGGING) {
                            // Handle moving selected region here
                            if(e.button.x >= p_win->clipboard_rect.x && 
                                    e.button.x <= p_win->clipboard_rect.x + p_win->clipboard_rect.w &&
                                    e.button.y >= p_win->clipboard_rect.y &&
                                    e.button.y <= p_win->clipboard_rect.y + p_win->clipboard_rect.h) {
                                SDL_Texture* p_temp_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, p_win->clipboard_rect.w, p_win->clipboard_rect.h);
                                draw(p_win->p_screen_renderer, NULL, p_temp_texture, NULL, NULL);
                                draw(p_win->p_screen_renderer, p_temp_texture, p_win->p_texture, NULL, &p_win->clipboard_rect);
                                draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
                                SDL_DestroyTexture(p_temp_texture);
                            } else {
                                p_win->mode = DRAWING;
                            }
                        } 
                        b_drawing = true;
                        start_x = e.button.x;
                        start_y = e.button.y;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    b_resizing = false;
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        b_drawing = false;
                        if(b_picking_color) {
                            b_picking_color = false;
                            break;
                        }
                        if(p_win->mode == BOX_DRAWING) {
                            SDL_FRect rect = get_rect(start_x, e.button.x, start_y, e.button.y);
                            draw_rect(p_win->p_screen_renderer, p_win->p_texture, &rect, color);
                        } else if(p_win->mode == SELECTING) {
                            SDL_FRect rect = get_rect(start_x, e.button.x, start_y, e.button.y);
                            SDL_DestroyTexture(p_win->p_clipboard_texture);
                            p_win->p_clipboard_texture = copy_texture(p_win->p_screen_renderer, p_win->p_texture, &rect);
                            p_win->clipboard_rect = rect;
                            p_win->mode = DRAGGING;
                        } else if(p_win->mode == DRAGGING) {
                            draw(p_win->p_screen_renderer, p_win->p_clipboard_texture, p_win->p_texture, NULL, &p_win->clipboard_rect);
                            draw(p_win->p_screen_renderer, p_win->p_texture, NULL, NULL, NULL);
                            SDL_Texture* p_temp_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
                            if(p_temp_texture == NULL) {
                                printf("Failed to create select box indicator texture: SDL_Error %s\n", SDL_GetError());
                                close_sdl(p_win);
                            }
                            draw(p_win->p_screen_renderer, p_win->p_texture, p_temp_texture, NULL, NULL);
                            draw_select_box(p_win->p_screen_renderer, 
                                    p_temp_texture, 
                                    p_win->clipboard_rect.x, 
                                    p_win->clipboard_rect.x+p_win->clipboard_rect.w, 
                                    p_win->clipboard_rect.y, 
                                    p_win->clipboard_rect.y+p_win->clipboard_rect.h);
                        }
                        add_action(p_win);
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    b_resizing = false;
                    if (!b_drawing) {
                        break;
                    }
                    if(p_win->mode == BOX_DRAWING) {
                        SDL_Texture* p_temp_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
                        if(p_temp_texture == NULL) {
                            printf("Failed to create box indicator texture: SDL_Error %s\n", SDL_GetError());
                            close_sdl(p_win);
                        }
                        draw(p_win->p_screen_renderer, p_win->p_texture, p_temp_texture, NULL, NULL);
                        SDL_FRect rect = get_rect(start_x, e.motion.x, start_y, e.motion.y);
                        draw_rect(p_win->p_screen_renderer, p_temp_texture, &rect, color);
                        SDL_DestroyTexture(p_temp_texture);
                        break;
                    } else if(p_win->mode == SELECTING) {
                        SDL_Texture* p_temp_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
                        if(p_temp_texture == NULL) {
                            printf("Failed to create select box indicator texture: SDL_Error %s\n", SDL_GetError());
                            close_sdl(p_win);
                        }
                        draw(p_win->p_screen_renderer, p_win->p_texture, p_temp_texture, NULL, NULL);
                        draw_select_box(p_win->p_screen_renderer, p_temp_texture, start_x, e.motion.x, start_y, e.motion.y);
                        SDL_DestroyTexture(p_temp_texture);
                        break;
                    } else if(p_win->mode == DRAGGING) {
                        SDL_Texture* p_temp_texture = SDL_CreateTexture(p_win->p_screen_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, texture_width, texture_height);
                        if(p_temp_texture == NULL) {
                            printf("Failed to create select box indicator texture: SDL_Error %s\n", SDL_GetError());
                            close_sdl(p_win);
                        }
                        int center_x = e.motion.x-p_win->clipboard_rect.w/2;
                        int center_y = e.motion.y-p_win->clipboard_rect.h/2;
                        SDL_FRect dst_rect = {center_x, center_y, p_win->clipboard_rect.w, p_win->clipboard_rect.h};
                        draw(p_win->p_screen_renderer, p_win->p_texture, p_temp_texture, NULL, NULL);
                        draw(p_win->p_screen_renderer, p_win->p_clipboard_texture, p_temp_texture, NULL, &dst_rect);
                        draw_select_box(p_win->p_screen_renderer, 
                                p_temp_texture, 
                                center_x, 
                                e.motion.x+p_win->clipboard_rect.w/2, 
                                center_y, 
                                e.motion.y+p_win->clipboard_rect.h/2);
                        p_win->clipboard_rect = dst_rect;
                        SDL_DestroyTexture(p_temp_texture);
                    } else if(p_win->mode == ERASING) {
                        const RGBA color_erase = {0x44, 0x44, 0x44, 255};
                        draw_circle(p_win->p_screen_renderer, p_win->p_texture, e.motion.x, e.motion.y, 40, color_erase);
                    } else {
                        draw_line(p_win->p_screen_renderer, p_win->p_texture, start_x, e.motion.x, start_y, e.motion.y, color);
                    }
                    if(p_win->mode == DRAWING) {
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
                        p_win->mode = DRAWING;
                    } else if(e.key.key == SDLK_D) {
                        p_win->mode = DRAWING;
                    } else if(e.key.key == SDLK_W) {
                        p_win->mode = DRAWING_NO_UPDATE;
                    } else if(e.key.key == SDLK_B) {
                        p_win->mode = BOX_DRAWING;
                    } else if(e.key.key == SDLK_S) {
                        p_win->mode = SELECTING;
                    } else if(e.key.key == SDLK_E) {
                        p_win->mode = ERASING;
                    } else if(e.key.key == SDLK_C) {
                        b_picking_color = true;
                        if(p_win->mode == SELECTING || p_win->mode == DRAGGING) {
                            p_win->mode = DRAWING;
                        }
                        float cursor_x, cursor_y;
                        SDL_GetMouseState(&cursor_x, &cursor_y);
                        SDL_Point cursor_pos = {cursor_x, cursor_y};
                        draw_color_picker(p_win->p_screen_renderer, p_win->p_texture, cursor_pos);
                        picker_x = cursor_x;
                        picker_y = cursor_y;
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
                            p_win->mode = DRAWING;
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
