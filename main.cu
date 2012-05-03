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

const unsigned a = 4096;
const unsigned b = 4096;

ndarray<float, 2> array1(a, b);
ndarray<float, 2> array2(a, b);

ndarray<float, 2> array3(a, b);

#if 0
template <typename T>
class add_inc {
public:
    inline
    void operator()(T &out, const T &in1, const T &in2) const
    {
        out = in1 + in2;
        out++;
    }

    static add_inc impl;
};
#endif

int main(int argc, char *argv[])
{
    for (unsigned i = 0; i < a; i++) {
        for (unsigned j = 0; j < b; j++) {
            array1[i][j] = i * 10 + j;
            array2[i][j] = i * 20 + j;
        }
    }

    compute(a, b,
        [&array3, &array1, &array2](unsigned y, unsigned x)
        {
            array3[y][x] = array1[y][x] + array2[y][x] + 1.f;
        });

    float accum;
    compute(a, b,
        [&accum, &array3](unsigned y, unsigned x)
        {
            accum += array3[y][x];
        });


    printf("Total: %f\n", accum);

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
