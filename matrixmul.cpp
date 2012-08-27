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

#include <chrono>
#include <iostream>
#include <string>

#include <map-reduce/map>
#include <map-reduce/array>
#include <map-reduce/dynarray>
#include <map-reduce/reduce>

#include <boost/multi_array.hpp>

#define C1 2.3f
#define C2 3.2f
#define C3 1.7f
#define C4 4.9f

using namespace map_reduce;

void print_banner(const std::string &name)
{
    std::cout << std::string(name.size(), '=') << std::endl;
    std::cout << name << std::endl;
    std::cout << std::string(name.size(), '=') << std::endl;
}

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
        make_range<int>(N));

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
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    std::cout << "=========" << std::endl;
    std::cout << "Reduction" << std::endl;
    std::cout << "=========" << std::endl;

    test_reduction_sum<1>();
    test_reduction_sum<100>();
    test_reduction_sum<10000>();

    end = std::chrono::system_clock::now();

    std::cout << "Sum: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();

    test_reduction_sum_2d<1,1>();
    test_reduction_sum_2d<1,10000>();
    test_reduction_sum_2d<100,1>();
    test_reduction_sum_2d<100,10000>();
    test_reduction_sum_2d<10000,1>();
    test_reduction_sum_2d<10000,10000>();

    end = std::chrono::system_clock::now();

    std::cout << "Sum 2D: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();

    test_reduction_max<1>();
    test_reduction_max<100>();
    test_reduction_max<10000>();

    end = std::chrono::system_clock::now();

    std::cout << "Max: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();

    test_reduction_max_2d<1,1>();
    test_reduction_max_2d<1,10000>();
    test_reduction_max_2d<100,1>();
    test_reduction_max_2d<100,10000>();
    test_reduction_max_2d<10000,1>();
    test_reduction_max_2d<10000,10000>();

    end = std::chrono::system_clock::now();

    std::cout << "Max 2D: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;
}

template <typename T, size_t N, size_t M>
void print(const array<T[N][M]> &a)
{
    for (unsigned i = 0; i < N; ++i) {
        for (unsigned j = 0; j < M; ++j) {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

template <typename T>
void test_matrixmul_gold(T &c, const T &a, const T &b, size_t N, size_t M)
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

enum class matrixmul_impl {
    pure,
    map,
    map_reduce
};

template <matrixmul_impl Impl, typename T, size_t N, size_t M>
size_t test_matrixmul_static_instance(T &c, T &a, T &b)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            b[i][j] = N * i + j + 1;
        },
        make_range(N, M));

    start = std::chrono::system_clock::now();

    if (Impl == matrixmul_impl::pure) {
        for (unsigned i = 0; i < N; ++i) {
            for (unsigned j = 0; j < M; ++j) {
                long tmp = 0;
                for (unsigned k = 0; k < N; ++k) {
                    tmp += a[i][k] * b[k][j];
                }
                c[i][j] = tmp;
            }
        }
    }

    if (Impl == matrixmul_impl::map) {
        map([&](int i, int j)
            {
                long tmp = 0;
                for (unsigned k = 0; k < N; ++k) {
                    tmp += a[i][k] * b[k][j];
                }
                c[i][j] = tmp;
            },
            make_range(N, M));
    }

    if (Impl == matrixmul_impl::map_reduce) {
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
    }

    end = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template <matrixmul_impl Impl, typename T>
size_t test_matrixmul_dyn_instance(T &c, T &a, T &b, size_t N, size_t M)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            b[i][j] = N * i + j + 1;
        },
        make_range(N, M));

    start = std::chrono::system_clock::now();

    if (Impl == matrixmul_impl::pure) {
        for (unsigned i = 0; i < N; ++i) {
            for (unsigned j = 0; j < M; ++j) {
                long tmp = 0;
                for (unsigned k = 0; k < N; ++k) {
                    tmp += a[i][k] * b[k][j];
                }
                c[i][j] = tmp;
            }
        }
    }

    if (Impl == matrixmul_impl::map) {
        map([&](int i, int j)
            {
                long tmp = 0;
                for (unsigned k = 0; k < N; ++k) {
                    tmp += a[i][k] * b[k][j];
                }
                c[i][j] = tmp;
            },
            make_range(N, M));
    }

    if (Impl == matrixmul_impl::map_reduce) {
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
    }

    end = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template <size_t N>
void test_matrixmul_static()
{
    print_banner("Matrixmul static");

    long (&a)[N][N] = *(long (*)[N][N]) new long[N * N];
    long (&b)[N][N] = *(long (*)[N][N]) new long[N * N];
    long (&c)[N][N] = *(long (*)[N][N]) new long[N * N];

    auto usecs = test_matrixmul_static_instance<matrixmul_impl::pure, long (&)[N][N], N, N>(c, a, b);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_static_instance<matrixmul_impl::map, long (&)[N][N], N, N>(c, a, b);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_static_instance<matrixmul_impl::map_reduce, long (&)[N][N], N, N>(c, a, b);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;
}

template <size_t N>
void test_matrixmul()
{
    print_banner("Matrix matrixmul");

    array<long[N][N]> a;
    array<long[N][N]> b;
    array<long[N][N]> c;

    auto usecs = test_matrixmul_static_instance<matrixmul_impl::pure, array<long[N][N]>, N, N>(c, a, b);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_static_instance<matrixmul_impl::map, array<long[N][N]>, N, N>(c, a, b);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_static_instance<matrixmul_impl::map_reduce, array<long[N][N]>, N, N>(c, a, b);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

template <size_t N>
void test_matrixmul_dyn()
{
    print_banner("Matrix matrixmul (dynarray)");

    using array_type = dynarray<long, 2>;
    array_type a(N, N);
    array_type b(N, N);

    array_type c(N, N), c_gold(N, N);

    auto usecs = test_matrixmul_dyn_instance<matrixmul_impl::pure, array_type>(c, a, b, N, N);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_dyn_instance<matrixmul_impl::map, array_type>(c, a, b, N, N);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_dyn_instance<matrixmul_impl::map_reduce, array_type>(c, a, b, N, N);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

template <size_t N>
void test_matrixmul_boost()
{
    print_banner("Matrix matrixmul (boost)");

    typedef boost::multi_array<long, 2> array_type;

    array_type a(boost::extents[N][N]);
    array_type b(boost::extents[N][N]);
    array_type c(boost::extents[N][N]);

    auto usecs = test_matrixmul_dyn_instance<matrixmul_impl::pure, array_type>(c, a, b, N, N);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_dyn_instance<matrixmul_impl::map, array_type>(c, a, b, N, N);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_matrixmul_dyn_instance<matrixmul_impl::map_reduce, array_type>(c, a, b, N, N);
    std::cout << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

void test_matrixmul()
{
    test_matrixmul_static<100>();
    test_matrixmul<100>();
    test_matrixmul_dyn<100>();
    test_matrixmul_boost<100>();

    test_matrixmul_static<500>();
    test_matrixmul<500>();
    test_matrixmul_dyn<500>();
    test_matrixmul_boost<500>();

    test_matrixmul_static<1000>();
    test_matrixmul<1000>();
    test_matrixmul_dyn<1000>();
    test_matrixmul_boost<1000>();
}

int main(int argc, char *argv[])
{
    test_matrixmul();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
