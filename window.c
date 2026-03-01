#include<stdlib.h>
#include<stdio.h>
#include"window.h"
#include"draw.h"

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
    Window* p_win = calloc(1,sizeof(Window));
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

void pop_action(Action** p_p_head, int* p_stack_size) {
    Action* p_temp = (*p_p_head)->p_prev;
    SDL_DestroyTexture((*p_p_head)->p_data);
    free(*p_p_head);
    *p_p_head = p_temp;
    --*p_stack_size;
}

void add_action(Window* p_win) {
    Action* p_new_action = calloc(1,sizeof(Action));
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
