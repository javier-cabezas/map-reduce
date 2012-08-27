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

#include "common.hpp"

#define C1 2.3f
#define C2 3.2f
#define C3 1.7f
#define C4 4.9f

using namespace map_reduce;

enum class stencil_impl {
    pure,
    map,
    map_reduce
};

template <int Order, stencil_impl Impl, typename T, size_t N, size_t M, bool Test = false>
size_t test_stencil_static_instance(T &a, T &b, T &c)
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

    if (Impl == stencil_impl::pure) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                long tmp = a[i][j];
                for (int k = 1; k <= Order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                if (Test) {
                    c[i][j] = tmp;
                } else {
                    b[i][j] = tmp;
                }
            }
        }
    }

    if (Impl == stencil_impl::map) {
        map([&](int i, int j)
            {
                long tmp = a[i][j];
                for (int k = 1; k <= Order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>({ Order, int(N) - Order}),
                       dim<int>({ Order, int(M) - Order})));
    }

    if (Impl == stencil_impl::map_reduce) {
        map([&](int i, int j)
            {
                b[i][j] = a[i][j] + reduce([&](int k)
                                           {
                                                return a[i - k][j] + a[i + k][j] +
                                                       a[i][j - k] + a[i][j + k];
                                           },
                                           reduce_ops<long>::add,
                                           make_range(dim<int>({1, Order + 1})));

            },
            make_range(dim<int>({ Order, int(N) - Order}),
                       dim<int>({ Order, int(M) - Order})));
    }

    end = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template <int Order, stencil_impl Impl, typename T, bool Test = false>
size_t test_stencil_dyn_instance(T &a, T &b, T &c, size_t N, size_t M)
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

    if (Impl == stencil_impl::pure || Test) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                long tmp = a[i][j];
                for (int k = 1; k <= Order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                if (Test) {
                    c[i][j] = tmp;
                } else {
                    b[i][j] = tmp;
                }
            }
        }
    }

    if (Impl == stencil_impl::map || Test) {
        map([&](int i, int j)
            {
                long tmp = a[i][j];
                for (int k = 1; k <= Order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>({ Order, int(N) - Order}),
                       dim<int>({ Order, int(M) - Order})));
    }

    if (Test) {
        assert(b == c);
    }

    if (Impl == stencil_impl::map_reduce || Test) {
        map([&](int i, int j)
            {
                b[i][j] = a[i][j] + reduce([&](int k)
                                           {
                                                return a[i - k][j] + a[i + k][j] +
                                                       a[i][j - k] + a[i][j + k];
                                           },
                                           reduce_ops<long>::add,
                                           make_range(dim<int>({1, Order + 1})));

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
void test_stencil_static()
{
    print_banner("Stencil static");

    long (&a)[N][N] = *(long (*)[N][N]) new long[N * N];
    long (&b)[N][N] = *(long (*)[N][N]) new long[N * N];
    long (&c)[N][N] = *(long (*)[N][N]) new long[N * N];

    auto usecs = test_stencil_static_instance<Order, stencil_impl::pure, long (&)[N][N], N, N>(a, b, c);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_stencil_static_instance<Order, stencil_impl::map, long (&)[N][N], N, N>(a, b, c);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_stencil_static_instance<Order, stencil_impl::map_reduce, long (&)[N][N], N, N>(a, b, c);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;
}

template <size_t Order, size_t N>
void test_stencil()
{
    print_banner("Matrix stencil");

    array<long[N][N]> a;
    array<long[N][N]> b;
    array<long[N][N]> c;

    auto usecs = test_stencil_static_instance<Order, stencil_impl::pure, array<long[N][N]>, N, N>(a, b, c);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_stencil_static_instance<Order, stencil_impl::map, array<long[N][N]>, N, N>(a, b, c);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_stencil_static_instance<Order, stencil_impl::map_reduce, array<long[N][N]>, N, N>(a, b, c);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

template <size_t Order, size_t N>
void test_stencil_dyn()
{
    print_banner("Matrix stencil (dynarray)");

    using array_type = dynarray<long, 2>;
    array_type a(N, N);
    array_type b(N, N);

    array_type c(N, N), c_gold(N, N);

    auto usecs = test_stencil_dyn_instance<Order, stencil_impl::pure, array_type>(a, b, c, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_stencil_dyn_instance<Order, stencil_impl::map, array_type>(a, b, c, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_stencil_dyn_instance<Order, stencil_impl::map_reduce, array_type>(a, b, c, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

template <size_t Order, size_t N>
void test_stencil_boost()
{
    print_banner("Matrix stencil (boost)");

    typedef boost::multi_array<long, 2> array_type;

    array_type a(boost::extents[N][N]);
    array_type b(boost::extents[N][N]);
    array_type c(boost::extents[N][N]);

    auto usecs = test_stencil_dyn_instance<Order, stencil_impl::pure, array_type>(a, b, c, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    usecs = test_stencil_dyn_instance<Order, stencil_impl::map, array_type>(a, b, c, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;

    test_stencil_dyn_instance<Order, stencil_impl::map_reduce, array_type>(a, b, c, N, N);
    std::cout << "[" << Order << "] " << N << "x" << N << ": " << usecs << " usecs " << std::endl;
}

void test_stencil()
{
    test_stencil_static<4, 100>();
    test_stencil<4, 100>();
    test_stencil_dyn<4, 100>();
    test_stencil_boost<4, 100>();

    test_stencil_static<4, 1000>();
    test_stencil<4, 1000>();
    test_stencil_dyn<4, 1000>();
    test_stencil_boost<4, 1000>();

    test_stencil_static<4, 10000>();
    test_stencil<4, 10000>();
    test_stencil_dyn<4, 10000>();
    test_stencil_boost<4, 10000>();

    test_stencil_static<8, 100>();
    test_stencil<8, 100>();
    test_stencil_dyn<8, 100>();
    test_stencil_boost<8, 100>();

    test_stencil_static<8, 1000>();
    test_stencil<8, 1000>();
    test_stencil_dyn<8, 1000>();
    test_stencil_boost<8, 1000>();

    test_stencil_static<8, 10000>();
    test_stencil<8, 10000>();
    test_stencil_dyn<8, 10000>();
    test_stencil_boost<8, 10000>();
}

int main(int argc, char *argv[])
{
    test_stencil();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
