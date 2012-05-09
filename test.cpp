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
#include "map"
#include "reduce"

#include "ndarray"

#define C1 2.3f
#define C2 3.2f
#define C3 1.7f
#define C4 4.9f

int func(int x, int y)
{
    if (x > y) return x;
    return y;
}

int main(int argc, char *argv[])
{
    static const unsigned N = 1000;
    static const unsigned M = 1000;

    array<int[N][M]> a;
    array<int[N][M]> b;

    array<int[N][M]> c;

    map([&](int i, int j)
        {
            if (i == N/2 && j == M/2) {
                c[i][j] = 6969;
            } else {
                c[i][j] = i;
            }

            a[i][j] = i;
            b[i][j] = j;
        },
        range<2>(N, M));

//#define USE_SPECIALIZED
//#define USE_LAMBDA

#ifdef USE_SPECIALIZED
    int max = reduce_max([&](int i, int j)
                         {
                             return c[i][j];
                         },
                         make_range(N, M));
#else
    int max = reduce([&](int i, int j)
                     {
                         return c[i][j];
                     },
#ifdef USE_LAMBDA
                     [&](int val, int tmp)
                     {
                         if (val > tmp) return val;
                         return tmp;
                     },
#else
                     reduce_ops<int>::greater_than,
#endif
                     make_range(N, M));
#endif

    map([&](int i, int j)
        {
            int tmp = 0;
            for (unsigned k = 0; k < N; ++k) {
                tmp = a[i][k] * b[k][j];
            }
            c[i][j] = tmp;
        },
        make_range(N, M),
        map_sched::parallel<1>());

    printf("MAX: %d\n", max);

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
