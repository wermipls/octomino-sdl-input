#include <SDL2/SDL.h>
#include <stdio.h>
#include "gui_renderer.h"
#include "microui.h"
#include "gui.h"
#include "sdl_input.h"
#include "config.h"

#define LOG_SIZE 64000

static  char logbuf[LOG_SIZE];
static   int logbuf_updated = 0;
static float bg[3] = { 90, 95, 100 };

static int window_open = 0;
static mu_Context *context;
static char not_available[] = "N/A";

void write_log(char *text) {
    if (logbuf[0]) { strcat(logbuf, "\n"); }

    if ((strlen(logbuf) + strlen(text) + 1) >= LOG_SIZE) {
        memcpy(logbuf, logbuf+LOG_SIZE/2, LOG_SIZE/2);
    }

    strcat(logbuf, text);
    logbuf_updated = 1;
}

static int uint_slider(mu_Context *ctx, unsigned int *value, int low, int high) {
    static float tmp;
    mu_push_id(ctx, &value, sizeof(value));
    tmp = *value;
    int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
    *value = tmp;
    mu_pop_id(ctx);
    return res;
}

static const char *get_con_buttonaxis_name(enum ButtonAxis ba)
{
    static const char names[][64] = {
        "Not set",
        "A",
        "B",
        "X",
        "Y",
        "Back",
        "Guide",
        "Start",
        "Left Stick Button",
        "Right Stick Button",
        "Left Shoulder",
        "Right Shoulder",
        "D-Pad Up",
        "D-Pad Down",
        "D-Pad Left",
        "D-Pad Right",
        "Left Stick +X",
        "Left Stick +Y",
        "Right Stick +X",
        "Right Stick +Y",
        "Left Stick -X",
        "Left Stick -Y",
        "Right Stick -X",
        "Right Stick -Y",
        "Left Trigger",
        "Right Trigger",
    };

    return names[ba];
}

static void log_panel(mu_Context *ctx) {
    if (mu_header(ctx, "Log")) {
        /* output text panel */
        mu_layout_row(ctx, 1, (int[]) { -1 }, 300);
        mu_begin_panel(ctx, "Log Output");
        mu_Container *panel = mu_get_current_container(ctx);
        mu_layout_row(ctx, 1, (int[]) { -1 }, -1);
        mu_text(ctx, logbuf);
        if (logbuf_updated) {
            panel->scroll.y = panel->content_size.y;
            logbuf_updated = 0;
        }
        mu_end_panel(ctx);
    }
}

static void coninfo_panel(mu_Context *ctx)
{
    if (mu_header_ex(ctx, "Controller info", MU_OPT_EXPANDED)) {
        const int widths[] = {150, -1};
        mu_layout_row(ctx, 2, widths, 0);

        
        mu_label(ctx,"Name");
        if (con != NULL) {
            const char *name = SDL_GameControllerName(con);
            if (name != NULL) {
                mu_label(ctx, name);
            } 
        } else {
            mu_label(ctx, not_available);
        }

        mu_label(ctx,"GameController mapping");
        if (con != NULL) {
            char *mapping = SDL_GameControllerMapping(con);
            if (mapping != NULL) {
                mu_label(ctx, mapping);
                SDL_free(mapping);
            } 
        } else {
            mu_label(ctx, not_available);
        }
    }
}

static void analog_panel(mu_Context *ctx, ControllerConfig *cfg)
{
    if (mu_begin_treenode_ex(ctx, "Analog stick", MU_OPT_EXPANDED)) {
        const int widths[] = {150, -1};
        mu_layout_row(ctx, 2, widths, 0);

        // deadzone slider
        mu_label(ctx, "Deadzone");
        float *dz = &cfg->deadzone;
        mu_slider(ctx, dz, 0.f, 1.f);

        // outer edge
        mu_label(ctx, "Outer edge");
        float *edge = &cfg->outer_edge;
        mu_slider(ctx, edge, 0.f, 1.f);

        // range
        mu_label(ctx, "Range");
        unsigned int *range = &cfg->range;
        uint_slider(ctx, range, 0, 127);

        // clamping checkbox
        mu_label(ctx, "Range clamping");
        int *clamp = &cfg->is_clamped;
        mu_checkbox(ctx, "", clamp);

        mu_end_treenode(ctx);
    }
}

static enum ButtonAxis *popup_ba;
static const char popup_name[] = "Binding Popup";

static void open_binding_popup(mu_Context *ctx, enum ButtonAxis *ba)
{
    popup_ba = ba;

    mu_Container *cnt = mu_get_container(ctx, popup_name);
    /* set as hover root so popup isn't closed in begin_window_ex()  */
    ctx->hover_root = ctx->next_hover_root = cnt;
    /* set position, open and bring-to-front */
    cnt->rect = mu_rect(150, 150, 1, 1);
    cnt->open = 1;
    mu_bring_to_front(ctx, cnt);
}

static void binding_popup(mu_Context *ctx)
{
    if (mu_begin_popup(ctx, popup_name)) {
        mu_Container *popup = mu_get_current_container(ctx);

        const int widths[] = {125, 125};
        mu_layout_row(ctx, 2, widths, 0);

        for (enum ButtonAxis i = CONTROLLER_NOT_SET; i < CONTROLLER_ENUM_END; ++i) {
            const char *label = get_con_buttonaxis_name(i);
            if (mu_button_ex_id(ctx, label, i, 0, MU_OPT_ALIGNCENTER)) {
                *popup_ba = i;
                popup->open = 0;
            }
        }
        mu_end_popup(ctx);
    }
}

static void binding_row(mu_Context *ctx, const char name[], ControllerMapping *mapping)
{
    mu_label(ctx, name);

    const char *label_primary = get_con_buttonaxis_name(mapping->primary);
    if (mu_button_ex_id(ctx, label_primary, (int)&mapping->primary, 0, MU_OPT_ALIGNCENTER)) {
        open_binding_popup(ctx, &mapping->primary);
    }

    const char *label_secondary = get_con_buttonaxis_name(mapping->secondary);
    if (mu_button_ex_id(ctx, label_secondary, (int)&mapping->secondary, 0, MU_OPT_ALIGNCENTER)) {
        open_binding_popup(ctx, &mapping->secondary);
    }
}

static void binding_panel(mu_Context *ctx, ControllerConfig *cfg)
{
    if (mu_begin_treenode_ex(ctx, "Bindings", MU_OPT_EXPANDED)) {
        const int widths[] = {150, 125, 125};
        mu_layout_row(ctx, 3, widths, 0);

        mu_label(ctx, "");
        mu_draw_control_text(ctx, "Primary", mu_layout_next(ctx), MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);
        mu_draw_control_text(ctx, "Secondary", mu_layout_next(ctx), MU_COLOR_TEXT, MU_OPT_ALIGNCENTER);

        binding_row(ctx, "A", &cfg->a);
        binding_row(ctx, "B", &cfg->b);
        binding_row(ctx, "Z", &cfg->z);
        binding_row(ctx, "L", &cfg->l);
        binding_row(ctx, "R", &cfg->r);
        binding_row(ctx, "Start", &cfg->start);
        binding_row(ctx, "C-Up", &cfg->cup);
        binding_row(ctx, "C-Down", &cfg->cdown);
        binding_row(ctx, "C-Left", &cfg->cleft);
        binding_row(ctx, "C-Right", &cfg->cright);
        binding_row(ctx, "D-Pad Up", &cfg->dup);
        binding_row(ctx, "D-Pad Down", &cfg->ddown);
        binding_row(ctx, "D-Pad Left", &cfg->dleft);
        binding_row(ctx, "D-Pad Right", &cfg->dright);
        binding_row(ctx, "Analog Up", &cfg->up);
        binding_row(ctx, "Analog Down", &cfg->down);
        binding_row(ctx, "Analog Left", &cfg->left);
        binding_row(ctx, "Analog Right", &cfg->right);

        binding_popup(ctx);

        mu_end_treenode(ctx);
    }
}

static void a2d_panel(mu_Context *ctx, ControllerConfig *cfg)
{
    if (mu_begin_treenode_ex(ctx, "Analog to digital mapping", MU_OPT_EXPANDED)) {
        const int widths[] = {150, -1};
        mu_layout_row(ctx, 2, widths, 0);

        mu_label(ctx, "Activation threshold");
        float *dz = &cfg->a2d_threshold;
        mu_slider(ctx, dz, 0.f, 1.f);

        mu_end_treenode(ctx);
    }
}

static void controller_panel(mu_Context *ctx, ControllerConfig *cfg, const char name[], int opt)
{
    if (mu_header_ex(ctx, name, opt)) {
        binding_panel(ctx, cfg);
        a2d_panel(ctx, cfg);
        analog_panel(ctx, cfg);
    }
}

static void configfile_panel(mu_Context *ctx)
{
    if (mu_header_ex(ctx, "Configuration file", MU_OPT_EXPANDED)) {
        const int widths[] = {150, -1};
        mu_layout_row(ctx, 2, widths, 0);

        const char curfile[] = "Current file";
        mu_label(ctx, curfile);
        mu_text(ctx, configpath);

        const int widths2[] = {150, 125, 125};
        mu_layout_row(ctx, 3, widths2, 0);
        mu_label(ctx, "");
        // save
        if (mu_button(ctx, "Save config")) {
            config_save();
        }
        // load
        if (mu_button(ctx, "Reload config")) {
            config_load();
        }
    }
}

static void test_window(mu_Context *ctx) {
    /* do window */
    int opt = MU_OPT_NOINTERACT | MU_OPT_NOTITLE;
    if (mu_begin_window_ex(ctx, "Demo Window", mu_rect(0, 0, 600, 600), opt)) {
        coninfo_panel(ctx);
        controller_panel(ctx, &concfg, "Controller 1 settings", MU_OPT_EXPANDED);
        configfile_panel(ctx);
        log_panel(ctx);

        mu_end_window(ctx);
    }
}



static void process_frame(mu_Context *ctx) {
    mu_begin(ctx);
    test_window(ctx);
    mu_end(ctx);
}



static const char button_map[256] = {
    [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
    [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
    [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
    [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
    [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
    [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
    [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
    [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};


static int text_width(mu_Font font, const char *text, int len) {
    if (len == -1) { len = strlen(text); }
    return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
    return r_get_text_height();
}


void config_window() {
    if (window_open)
        return;

    window_open = 1;

    /* init renderer */
    r_init();

    /* init microui */
    context = malloc(sizeof(mu_Context));
    mu_init(context);
    context->text_width = text_width;
    context->text_height = text_height;

    /* main loop */
    for (;;) {
        /* handle SDL events */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: gui_deinit(); return;
                case SDL_MOUSEMOTION: mu_input_mousemove(context, e.motion.x, e.motion.y); break;
                case SDL_MOUSEWHEEL: mu_input_scroll(context, 0, e.wheel.y * -30); break;
                case SDL_TEXTINPUT: mu_input_text(context, e.text.text); break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    int b = button_map[e.button.button & 0xff];
                    if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(context, e.button.x, e.button.y, b); }
                    if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(context, e.button.x, e.button.y, b);   }
                    break;
                }

                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    int c = key_map[e.key.keysym.sym & 0xff];
                    if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(context, c); }
                    if (c && e.type ==   SDL_KEYUP) { mu_input_keyup(context, c);   }
                    break;
                }
            }
        }

        /* process frame */
        process_frame(context);

        /* render */
        r_clear(mu_color(bg[0], bg[1], bg[2], 255));
        mu_Command *cmd = NULL;
        while (mu_next_command(context, &cmd)) {
            switch (cmd->type) {
                case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
                case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
                case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
                case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
            }
        }
        r_present();
    }
}

void gui_deinit()
{
    window_open = 0;
    free(context);
    r_close();
}
