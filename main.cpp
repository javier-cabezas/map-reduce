/*
 * <+ DESCRIPTION +>
 *
 * Copyright (C) 2011, Javier Cabezas <jcabezas in ac upc edu> {{{
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either 
 * version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * }}}
 */

#include <cstdio>

#include "gmap"

const unsigned a = 1024;
const unsigned b = 1024;

#define USE_LAMBDA
//#define USE_NDARRAY

#ifdef USE_NDARRAY
ndarray<float, 2> array1(a, b);
ndarray<float, 2> array2(a, b);

ndarray<float, 2> array3(a, b);
#else
float array1[a][b];
float array2[a][b];

float array3[a][b];
#endif

int main(int argc, char *argv[])
{
#ifdef USE_LAMBDA
    independent(a, b,
        [&array1, &array2](unsigned y, unsigned x) -> void
        {
#else
    #pragma omp parallel for
    for (unsigned y = 0; y < a; y++) {
        for (unsigned x = 0; x < b; x++) {
#endif
            array1[y][x] = y * 10 + x;
            array2[y][x] = y * 20 + x;
#ifdef USE_LAMBDA
        });
#else
        }
    }
#endif

#ifdef USE_LAMBDA
    independent(a, b,
        [&array3, &array1, &array2](unsigned y, unsigned x) -> void
        {
#else
    #pragma omp parallel for
    for (unsigned y = 0; y < a; y++) {
        for (unsigned x = 0; x < b; x++) {
#endif
            float val = 0.f;
            for (unsigned i = 0; i < b; i++) {
                 val += array1[y][i] * array2[i][x];
            }
            array3[y][x] = val;
#ifdef USE_LAMBDA
        });
#else
        }
    }
#endif

    float accum = 0.f;
#ifdef USE_LAMBDA
    sequential(a, b,
        [&accum, &array3](unsigned y, unsigned x) -> void
        {
#else
    for (unsigned y = 0; y < a; y++) {
        for (unsigned x = 0; x < b; x++) {
#endif
        accum += array3[y][x];
#ifdef USE_LAMBDA
        });
#else
        }
    }
#endif

    printf("Total: %f\n", accum);

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
