/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "util.hpp"
#include <stdarg.h>
#include <time.h>
#include <limits.h>
#include <debugapi.h>

HINSTANCE g_hinstance;
CRITICAL_SECTION g_critical; 

FILE *logfile;

int16_t threshold(int16_t val, float cutoff)
{
    if (val < 0)
        return val >= -(cutoff * 32767) ? 0 : val;
    else
        return val <= (cutoff * 32767) ? 0 : val;
}

void scale_and_limit(int16_t *x, int16_t *y, float dz, float edge)
{
    // get abs value between 0 and 1 relative to deadzone and edge
    int16_t div = edge * 32767 - dz * 32767;
    if (div == 0) {
        return;
    }
    float fx = (abs(*x) - dz * 32767) / div;
    float fy = (abs(*y) - dz * 32767) / div;

    // out of range
    if (fx > 1.f) {
        fy = fy * (1.f / fx);
        fx = 1.f;
    }
    if (fy > 1.f) {
        fx = fx * (1.f / fy);
        fy = 1.f;
    }

    float sign_x = 0;
    float sign_y = 0;

    // deadzone
    if (*y != 0) {
        if (fy <= 0.f)
            fy = 0.f;
        else
            sign_y = abs(*y) / *y;
    }

    if (*x != 0) {
        if (fx <= 0.f)
            fx = 0.f;
        else 
            sign_x = abs(*x) / *x;
    }

    *x = sign_x * fx * 32767;
    *y = sign_y * fy * 32767;
}

int16_t sclamp(int16_t val, int16_t min, int16_t max)
{
    if (val <= min) return min;
    if (val >= max) return max;
    return val;
}

int16_t smin(int16_t val, int16_t min)
{
    if (val <= min) return min;
    return val;
}

int16_t smax(int16_t val, int16_t max)
{
    if (val >= max) return max;
    return val;
}

void dlog(const char *fmt, ...)
{
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    char buf[1024];

    char timestr[9];
    strftime(timestr, sizeof(timestr), "%X", timeinfo);

    fprintf(logfile, "[%s] ", timestr);

    va_list args;
    va_start(args, fmt);
    vfprintf(logfile, fmt, args);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    OutputDebugStringA(buf);

    fprintf(logfile, "\n");
}
