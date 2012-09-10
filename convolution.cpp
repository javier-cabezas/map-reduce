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
 * afloat with this program; if not, write to the Free Software
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

#include "common.hpp"

#define C1 2.3f
#define C2 3.2f
#define C3 1.7f
#define C4 4.9f

using namespace map_reduce;

enum class convolution_impl {
    pure,
    map,
    map_reduce
};

template <int Order, convolution_impl Impl, typename T1, typename T2, size_t N, size_t M, bool Test = DoTest>
size_t test_convolution_static_instance(T1 &a, T1 &b, T1 &c, const T2 &conv)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            if (Test) {
                b[i][j] = 0;
                c[i][j] = 0;
            }
        },
        make_range(N, M));

    fill_cache();

    start = std::chrono::system_clock::now();

    if (Impl == convolution_impl::pure || Test) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                float tmp = a[i][j];
                for (int k1 = 0; k1 < 2 * Order + 1; ++k1) {
                    for (int k2 = 0; k2 < 2 * Order + 1; ++k2) {
                        tmp += conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                    }
                }

                if (Test) {
                    c[i][j] = tmp;
                } else {
                    b[i][j] = tmp;
                }
            }
        }
    }

    if (Impl == convolution_impl::map) {
        map([&](int i, int j)
            {
                float tmp = a[i][j];
                for (int k1 = 0; k1 < 2 * Order + 1; ++k1) {
                    for (int k2 = 0; k2 < 2 * Order + 1; ++k2) {
                        tmp += conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                    }
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>({ Order, int(N) - Order}),
                       dim<int>({ Order, int(M) - Order})));
    }

    if (Test) {
        //assert(b == c);
    }

    if (Impl == convolution_impl::map_reduce) {
        map([&](int i, int j)
            {
                b[i][j] = a[i][j] + reduce([&](int k1, int k2)
                                           {
                                                return conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                                           },
                                           reduce_ops<float>::add,
                                           make_range(dim<int>(2 * Order + 1),
                                                      dim<int>(2 * Order + 1)));
            },
            make_range(dim<int>({ Order, int(N) - Order}),
                       dim<int>({ Order, int(M) - Order})));
    }

    if (Test) {
        //assert(b == c);
    }

    end = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template <int Order, convolution_impl Impl, typename T, bool Test = DoTest>
size_t test_convolution_dyn_instance(T &a, T &b, T &c, const T &conv, size_t N, size_t M)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            if (Test) {
                b[i][j] = 0;
                c[i][j] = 0;
            }
        },
        make_range(N, M));

    fill_cache();

    start = std::chrono::system_clock::now();

    if (Impl == convolution_impl::pure || Test) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                float tmp = a[i][j];
                for (int k1 = 0; k1 < 2 * Order + 1; ++k1) {
                    for (int k2 = 0; k2 < 2 * Order + 1; ++k2) {
                        tmp += conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                    }
                }

                if (Test) {
                    c[i][j] = tmp;
                } else {
                    b[i][j] = tmp;
                }
            }
        }
    }

    if (Impl == convolution_impl::map || Test) {
        map([&](int i, int j)
            {
                float tmp = a[i][j];
                for (int k1 = 0; k1 < 2 * Order + 1; ++k1) {
                    for (int k2 = 0; k2 < 2 * Order + 1; ++k2) {
                        tmp += conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                    }
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>({ Order, int(N) - Order}),
                       dim<int>({ Order, int(M) - Order})));
    }

    if (Test) {
        assert(b == c);
    }

    if (Impl == convolution_impl::map_reduce || Test) {
        map([&](int i, int j)
            {
                b[i][j] = a[i][j] + reduce([&](int k1, int k2)
                                           {
                                                return conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                                           },
                                           reduce_ops<float>::add,
                                           make_range(dim<int>(Order + 1),
                                                      dim<int>(Order + 1)));

            },
            make_range(dim<int>({ Order, int(N) - Order}),
                       dim<int>({ Order, int(M) - Order})));
    }

    if (Test) {
        assert(b == c);
    }

    end = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template <size_t Order, size_t N>
void test_convolution_static()
{
    print_banner("2D convolution (static)");

    float (&a)[N][N] = *(float (*)[N][N]) new float[N * N];
    float (&b)[N][N] = *(float (*)[N][N]) new float[N * N];
    float (&c)[N][N] = *(float (*)[N][N]) new float[N * N];

    float (&conv)[2 * Order + 1][2 * Order + 1] = *(float (*)[2 * Order + 1][2 * Order + 1]) new float[(2 * Order + 1) * (2 * Order + 1)];

    auto usecs = test_convolution_static_instance<Order,
                                                  convolution_impl::pure,
                                                  float (&)[N][N],
                                                  float (&)[2 * Order + 1][2 * Order + 1],
                                                  N, N>(a, b, c, conv);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_convolution_static_instance<Order,
                                             convolution_impl::map,
                                             float (&)[N][N],
                                             float (&)[2 * Order + 1][2 * Order + 1],
                                             N, N>(a, b, c, conv);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_convolution_static_instance<Order,
                                             convolution_impl::map_reduce,
                                             float (&)[N][N],
                                             float (&)[2 * Order + 1][2 * Order + 1],
                                             N, N>(a, b, c, conv);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;

    delete [] &conv;
}

template <size_t Order, size_t N>
void test_convolution()
{
    print_banner("2D convolution (array)");

    array<float[N][N]> a;
    array<float[N][N]> b;
    array<float[N][N]> c;

    array<float[2 * Order + 1][2 * Order + 1]> conv;

    auto usecs = test_convolution_static_instance<Order,
                                                  convolution_impl::pure,
                                                  array<float[N][N]>,
                                                  array<float[2 * Order + 1][2 * Order + 1]>,
                                                  N, N>(a, b, c, conv);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_convolution_static_instance<Order,
                                             convolution_impl::map,
                                             array<float[N][N]>,
                                             array<float[2 * Order + 1][2 * Order + 1]>,
                                             N, N>(a, b, c, conv);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_convolution_static_instance<Order,
                                             convolution_impl::map_reduce,
                                             array<float[N][N]>,
                                             array<float[2 * Order + 1][2 * Order + 1]>,
                                             N, N>(a, b, c, conv);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

template <size_t Order, size_t N>
void test_convolution_dyn()
{
    print_banner("2D convolution (dynarray)");

    using array_type = dynarray<float, 2>;
    array_type a(N, N);
    array_type b(N, N);
    array_type c(N, N);

    array_type conv(2 * Order + 1, 2 * Order + 1);

    auto usecs = test_convolution_dyn_instance<Order, convolution_impl::pure, array_type>(a, b, c, conv, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_convolution_dyn_instance<Order, convolution_impl::map, array_type>(a, b, c, conv, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_convolution_dyn_instance<Order, convolution_impl::map_reduce, array_type>(a, b, c, conv, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

template <size_t Order, size_t N>
void test_convolution_boost()
{
    print_banner("2D convolution (boost::multi_array)");

    typedef boost::multi_array<float, 2> array_type;

    array_type a(boost::extents[N][N]);
    array_type b(boost::extents[N][N]);
    array_type c(boost::extents[N][N]);

    array_type conv(boost::extents[2 * Order + 1][2 * Order + 1]);

    auto usecs = test_convolution_dyn_instance<Order, convolution_impl::pure, array_type>(a, b, c, conv, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_convolution_dyn_instance<Order, convolution_impl::map, array_type>(a, b, c, conv, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    test_convolution_dyn_instance<Order, convolution_impl::map_reduce, array_type>(a, b, c, conv, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

void test_convolution()
{
    test_convolution_static<1, 100>();
    test_convolution<1, 100>();
    test_convolution_dyn<1, 100>();
    test_convolution_boost<1, 100>();

    test_convolution_static<1, 1000>();
    test_convolution<1, 1000>();
    test_convolution_dyn<1, 1000>();
    test_convolution_boost<1, 1000>();

    test_convolution_static<1, 10000>();
    test_convolution<1, 10000>();
    test_convolution_dyn<1, 10000>();
    test_convolution_boost<1, 10000>();

    test_convolution_static<2, 100>();
    test_convolution<2, 100>();
    test_convolution_dyn<2, 100>();
    test_convolution_boost<2, 100>();

    test_convolution_static<2, 1000>();
    test_convolution<2, 1000>();
    test_convolution_dyn<2, 1000>();
    test_convolution_boost<2, 1000>();

    test_convolution_static<2, 10000>();
    test_convolution<2, 10000>();
    test_convolution_dyn<2, 10000>();
    test_convolution_boost<2, 10000>();
}

int main(int argc, char *argv[])
{
    test_convolution();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
