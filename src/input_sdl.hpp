/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OCTOMINO_SDL_INPUT_H_
#define OCTOMINO_SDL_INPUT_H_

#include <synchapi.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_version.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

extern CRITICAL_SECTION critical_section; 

extern FILE *logfile;
extern char dbpath[PATH_MAX];
extern SDL_GameController *con;
extern int initialized;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define SDL_VER_STRING \
    TOSTRING(SDL_MAJOR_VERSION) "." \
    TOSTRING(SDL_MINOR_VERSION) "." \
    TOSTRING(SDL_PATCHLEVEL)

#define PLUGIN_ABOUT   \
    PLUGIN_NAME \
    "\nVersion " PLUGIN_VERSION \
    "\nCompiled on " __DATE__ " using SDL " SDL_VER_STRING  \
    "\n\n" PLUGIN_REPO \
    "\n\nLicensed under the Mozilla Public License 2.0" \
    "\n(http://mozilla.org/MPL/2.0/)"

typedef struct inputs_t
{
    uint16_t a       : 1;
    uint16_t b       : 1;
    uint16_t x       : 1;
    uint16_t y       : 1;

    uint16_t back    : 1;
    uint16_t guide   : 1;
    uint16_t start   : 1;

    uint16_t lstick  : 1;
    uint16_t rstick  : 1;
    uint16_t lshoul  : 1;
    uint16_t rshoul  : 1;

    uint16_t dup     : 1;
    uint16_t ddown   : 1;
    uint16_t dleft   : 1;
    uint16_t dright  : 1;

    int16_t alx;
    int16_t aly;

    int16_t arx;
    int16_t ary;

    int16_t altrig;
    int16_t artrig;
} inputs_t;

int try_init(void);
void deinit(void);
void con_open(void);
void con_close(void);
int16_t threshold(int16_t val, float cutoff);
void scale_and_limit(int16_t *x, int16_t *y, float dz, float edge);
int16_t sclamp(int16_t val, int16_t min, int16_t max);
int16_t smin(int16_t val, int16_t min);
int16_t smax(int16_t val, int16_t max);
void con_get_inputs(inputs_t *i);

void con_write_inputs(inputs_t *i);
void dlog(const char *fmt, ...);

#endif
