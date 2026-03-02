#include<SDL3/SDL.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define ACTION_STACK_MAX_SIZE 64

typedef struct Action{
    SDL_Texture* p_data;
    struct Action* p_prev;
} Action; 

typedef enum Mode {
    DRAWING = 0,
    DRAWING_NO_UPDATE,
    BOX_DRAWING,
    SELECTING,
    DRAGGING,
    ERASING
} Mode;

typedef struct Window {
    SDL_Window* p_window;
    SDL_Renderer* p_screen_renderer;
    SDL_Texture* p_texture;
    SDL_Texture* p_clipboard_texture;
    SDL_FRect clipboard_rect;
    Action* p_action_head;
    int stack_size;
    Mode mode;
} Window;


// Handles closing the program, frees heap memory, and calls all the SDL destroy functions needed.
void close_sdl(Window* p_win);
// Creates the SDL_Window, SDL_Renderer then returns a pointer to a Window object containing them stored on the heap.
Window* create_window();
// Pop action from end of the linked list of actions.
void pop_action(Action** p_p_head, int* p_stack_size);
/* Add action to the end of the linked list of actions.
 * If we are at the action stack max then we add an action to the end then remove the first action in the list.
 */
void add_action(Window* p_win);
