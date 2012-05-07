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

#if 0

#include <cstdio>

#include "gmap"

const unsigned a = 1024;
const unsigned b = 1024;

static float array1[a][b];
static float array2[a][b];

static float array3[a][b];

#define USE_LAMBDA

int main(int argc, char *argv[])
{
#ifdef USE_LAMBDA
    independent(a, b,
        [&array1, &array2](unsigned y, unsigned x)
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
        [&array3, &array1, &array2](unsigned y, unsigned x)
        {
#else
    #pragma omp parallel for
    for (unsigned y = 0; y < a; y++) {
        for (unsigned x = 0; x < b; x++) {
#endif
            float val = 0.f;
            for (unsigned i = 0; i < a; i++) {
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
        [&accum, &array3](unsigned y, unsigned x)
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
#endif

#include <cassert>
#include <cstdio>

#define C1 2.3f
#define C2 3.2f
#define C3 1.7f
#define C4 4.9f

template <typename Func>
__global__
void kernel(Func f, float *a)
{
    f(threadIdx.x, threadIdx.y, a);
}

float a_host, *a_dev;

template <typename Func>
void map(Func f, int x, int y)
{
    if (1) {
        kernel<<<1, 1>>>(f, a_dev);
        cudaError_t err = cudaThreadSynchronize();
        assert(err == cudaSuccess);
    }
}

typedef void (*func)(int x, int y, float *a);

__device__
void func1(int x, int y, float *a)
{
    *a = 69.f;
}

__device__
func func1_p = func1;

int main(int argc, char *argv[])
{
    cudaError_t err = cudaMalloc(&a_dev, sizeof(float));
    assert(err == cudaSuccess);

    func f;
    cudaMemcpyFromSymbol(&f, func1_p, sizeof(func));

    map(f, 1000, 1000);

    err = cudaMemcpy(&a_host, a_dev, sizeof(float), cudaMemcpyDefault);
    assert(err == cudaSuccess);

    printf("%f\n", a_host);

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
