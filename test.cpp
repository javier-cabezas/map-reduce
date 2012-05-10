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

template <typename T>
int func_greater(T x, T y)
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
                 reduce_ops<long>::add,
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
    array<long[N]> a;

    map([&](int i)
        {
            if (i == N/2) {
                a[i] = NUM;
            } else {
                a[i] = i + 1;
            }
        },
        make_range(N));

    long max = reduce([&](int i)
                      {
                          return a[i];
                      },
                      [&](long a, long b)
                      {
                          return a > b? a: b;
                      },
                      make_range(N));

    assert(max == NUM);

    max = reduce([&](int i)
                 {
                     return a[i];
                 },
                 reduce_ops<long>::greater_than,
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

template <size_t N, size_t M>
int test_reduction_max_2d()
{
#define NUM 696969696
    array<long[N][M]> a;

    map([&](int i, int j)
        {
            if (i == N/2 && j == M/2) {
                a[i][j] = NUM;
            } else {
                a[i][j] = N * i + j +1;
            }
        },
        make_range(N, M));

    long max = reduce([&](int i, int j)
                     {
                         return a[i][j];
                     },
                     [](long a, long b)
                     {
                         return a > b? a: b;
                     },
                     make_range(N, M));

    assert(max == NUM);

    max = reduce([&](int i, int j)
                 {
                     return a[i][j];
                 },
                 reduce_ops<long>::greater_than,
                 make_range(N, M));

    assert(max == NUM);

    max = reduce_max([&](int i, int j)
                     {
                         return a[i][j];
                     },
                     make_range(N, M));

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
    
    test_reduction_max_2d<1,1>();
    test_reduction_max_2d<1,100>();
    test_reduction_max_2d<1,10000>();
    test_reduction_max_2d<100,1>();
    test_reduction_max_2d<100,100>();
    test_reduction_max_2d<100,10000>();
    test_reduction_max_2d<10000,1>();
    test_reduction_max_2d<10000,100>();
    test_reduction_max_2d<10000,10000>();
}

template <typename T, size_t N, size_t M>
void print(const array<T[N][M]> &a)
{
    for (unsigned i = 0; i < N; ++i) {
        for (unsigned j = 0; j < M; ++j) {
            printf("%lu ", a[i][j]);
        }
        printf("\n");
    }
}

template <typename T, size_t N, size_t M>
void test_matrixmul_gold(array<T[N][M]> &c, 
                         const array<T[N][M]> &a, 
                         const array<T[N][M]> &b)
{
    for (unsigned i = 0; i < N; ++i) {
        for (unsigned j = 0; j < M; ++j) {
            T tmp = 0;
            for (unsigned k = 0; k < N; ++k) {
                tmp += a[i][k] * b[k][j];
            }
            c[i][j] = tmp;
        }
    }
}

template <size_t N, size_t M>
int test_matrixmul_instance()
{
    array<long[N][M]> a;
    array<long[N][M]> b;

    array<long[N][M]> c, c_gold;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j +1;
            b[i][j] = N * i + j +1;
        },
        make_range(N, M));

    test_matrixmul_gold(c_gold, a, b);

    map([&](int i, int j)
        {
            long tmp = 0;
            for (unsigned k = 0; k < N; ++k) {
                tmp += a[i][k] * b[k][j];
            }
            c[i][j] = tmp;
        },
        make_range(N, M));

    assert(c == c_gold);

    map([&](int i, int j)
        {
            c[i][j] = reduce([&](int k)
                             {
                                 return a[i][k] * b[k][j];
                             },
                             reduce_ops<long>::add,
                             make_range(N));
        },
        make_range(N, M));

    assert(c == c_gold);

    return 0;
}

int test_matrixmul()
{
    test_matrixmul_instance<1, 1>();
    test_matrixmul_instance<10, 10>();
    test_matrixmul_instance<100, 100>();
    test_matrixmul_instance<1000, 1000>();

    return 0;
}

int test_ref()
{
    array<int[10][1]> a;
    array_ref<int[10][1]> a_ref(a);
    array_ref<int[1][10]> b_ref(a.reshape<int[1][10]>());

    return 0;
}

int main(int argc, char *argv[])
{
    test_reduction();
    test_matrixmul();
    test_ref();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
