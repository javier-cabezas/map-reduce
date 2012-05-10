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

#include <iostream>

#include <map-reduce/map>
#include <map-reduce/ndarray>
#include <map-reduce/reduce>

#define C1 2.3f
#define C2 3.2f
#define C3 1.7f
#define C4 4.9f

using namespace map_reduce;

int func(int x, int y)
{
    if (x > y) return x;
    return y;
}

template <size_t N>
int test_reduction_sum()
{
    array<long[N]> a;

    map([&](int i)
        {
            a[i] = i + 1;
        },
        make_range(N));

    long sum = reduce([&](int i)
                      {
                          return a[i];
                      },
                      [&](long a, long b)
                      {
                          return a + b;
                      },
                      make_range(N));

    assert(sum == (N + 1) * N / 2);

    sum = reduce([&](int i)
                 {
                     return a[i];
                 },
                 reduce_ops<int>::add,
                 make_range(N));

    assert(sum == (N + 1) * N / 2);

    sum = reduce_sum([&](int i)
                     {
                         return a[i];
                     },
                     make_range(N));

    assert(sum == (N + 1) * N / 2);

    return 0;
}

template <size_t N, size_t M>
int test_reduction_sum_2d()
{
    array<long[N][M]> a;
    map([&](int i, int j)
        {
            a[i][j] = i + 1;
        },
        make_range(N, M));

    long sum = reduce([&](int i, int j)
                     {
                         return a[i][j];
                     },
                     [&](long a, long b)
                     {
                         return a + b;
                     },
                     make_range(N, M));

    assert(sum == ((N + 1) * N / 2) * M);

    sum = reduce([&](int i, int j)
                 {
                     return a[i][j];
                 },
                 reduce_ops<long>::add,
                 make_range(N, M));

    assert(sum == ((N + 1) * N / 2) * M);

    sum = reduce_sum([&](int i, int j)
                     {
                         return a[i][j];
                     },
                     make_range(N, M));

    assert(sum == ((N + 1) * N / 2) * M);
}

template <size_t N>
int test_reduction_max()
{
#define NUM 696969696
    array<int[N]> a;

    map([&](int i)
        {
            if (i == N/2) {
                a[i] = int(NUM);
            } else {
                a[i] = i + 1;
            }
        },
        make_range(N));

    int max = reduce([&](int i)
                     {
                         return a[i];
                     },
                     [&](int a, int b)
                     {
                         return a > b? a: b;
                     },
                     make_range(N));

    assert(max == NUM);

    max = reduce([&](int i)
                 {
                     return a[i];
                 },
                 reduce_ops<int>::greater_than,
                 make_range(N));

    assert(max == NUM);

    max = reduce_max([&](int i)
                     {
                         return a[i];
                     },
                     make_range(N));

    assert(max == NUM);

    return 0;
}

int test_reduction()
{
    test_reduction_sum<1>();
    test_reduction_sum<10>();
    test_reduction_sum<100>();
    test_reduction_sum<1000>();
    test_reduction_sum<10000>();

    test_reduction_sum_2d<1,1>();
    test_reduction_sum_2d<1,100>();
    test_reduction_sum_2d<1,10000>();
    test_reduction_sum_2d<100,1>();
    test_reduction_sum_2d<100,100>();
    test_reduction_sum_2d<100,10000>();
    test_reduction_sum_2d<10000,1>();
    test_reduction_sum_2d<10000,100>();
    test_reduction_sum_2d<10000,10000>();

    test_reduction_max<1>();
    test_reduction_max<10>();
    test_reduction_max<100>();
    test_reduction_max<1000>();
    test_reduction_max<10000>();
}

int main(int argc, char *argv[])
{
    test_reduction();

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

    int max = reduce_max([&](int i, int j)
                         {
                             return c[i][j];
                         },
                         make_range(N, M));

    max = reduce([&](int i, int j)
                 {
                     return c[i][j];
                 },
                 [&](int val, int tmp)
                 {
                     if (val > tmp) return val;
                     return tmp;
                 },
                 make_range(N, M), reduce_sched::serial());

    max = reduce([&](int i, int j)
                 {
                     return c[i][j];
                 },
                 reduce_ops<int>::greater_than,
                 make_range(N, M), reduce_sched::serial());

    map([&](int i, int j)
        {
            int tmp = 0;
            for (unsigned k = 0; k < N; ++k) {
                tmp = a[i][k] * b[k][j];
            }
            c[i][j] = tmp;
        },
        make_range(N, M));

    map([&](int i, int j)
        {
            c[i][j] = reduce([&](int k)
                             {
                                 return a[i][k] * b[k][j];
                             },
                             reduce_ops<int>::add,
                             make_range(N),
                             reduce_sched::serial());
        },
        make_range(N, M));

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
