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
#include <gmap>

#define C1 2.3f
#define C2 3.2f
#define C3 1.7f
#define C4 4.9f

void func(int x, int y)
{
}

int main(int argc, char *argv[])
{
    ref<int[1000][1000]> a = make_array<int[1000][1000]>();
    ref<int[1000][1000]> b = make_array<int[1000][1000]>();

    map(func,
        1000, 1000);

    map([&](int x, int y)
        {
            if (x == 500) {
                a[x][y] = 6969;
            } else {
                a[x][y] = x;
            }
            b[x][y] = y;
        },
        1000, 1000);

    int max = reduce(
           [&](int x, int y)
           {
               return a[x][y];
           },
           [&](int val, int tmp)
           {
               if (val > tmp) return val;
               return tmp;
           },
           0,
           1000, 1000);

    map([&](int x, int y)
        {
            int tmp = 0;
            for (unsigned i = 0; i < 1000; ++i) {
                tmp = a[x][i] * b[i][y];
            }
            a[x][y] = tmp;
        },
        1000, 1000);

    printf("MAX: %d\n", max);

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
