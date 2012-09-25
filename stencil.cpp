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
 * adata_type with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * }}}
 */

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

template <int Order, stencil_impl Impl, typename T, size_t N, size_t M, bool Test = DoTest>
size_t test_stencil_static_instance(T &a, T &b, T &c)
{
    my_time_point start, end;

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

    start = my_clock::now();

    if (Impl == stencil_impl::pure || Test) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                data_type tmp = a[i][j];
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
                data_type tmp = a[i][j];
                for (int k = 1; k <= Order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
    }

    if (Test) {
        // assert(b == c);
    }

    if (Impl == stencil_impl::map_reduce) {
        map([&](int i, int j)
            {
                b[i][j] = a[i][j] + reduce([&](int k)
                                           {
                                                return a[i - k][j] + a[i + k][j] +
                                                       a[i][j - k] + a[i][j + k];
                                           },
                                           reduce_ops<data_type>::add,
                                           make_range(dim<int>(1, Order + 1)));

            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
    }

    if (Test) {
        // assert(b == c);
    }

    end = my_clock::now();

    return microsecond_cast(end - start).count();
}

template <int Order, stencil_impl Impl, typename T, bool Test = DoTest>
size_t test_stencil_dyn_instance(T &a, T &b, T &c, size_t N, size_t M)
{
    my_time_point start, end;

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

    start = my_clock::now();

    if (Impl == stencil_impl::pure || Test) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                data_type tmp = a[i][j];
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
                data_type tmp = a[i][j];
                for (int k = 1; k <= Order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
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
                                           reduce_ops<data_type>::add,
                                           make_range(dim<int>({1, Order + 1})));

            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
    }

    if (Test) {
        assert(b == c);
    }

    end = my_clock::now();

    return microsecond_cast(end - start).count();
}

template <size_t Order, size_t N>
void test_stencil_static()
{
    data_type (&a)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];
    data_type (&b)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];
    data_type (&c)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];

    std::cout << "S:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_static_instance<Order, stencil_impl::pure, data_type (&)[N][N], N, N>(a, b, c);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_static_instance<Order, stencil_impl::map, data_type (&)[N][N], N, N>(a, b, c);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_static_instance<Order, stencil_impl::map_reduce, data_type (&)[N][N], N, N>(a, b, c);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;
}

template <size_t Order, size_t N>
void test_stencil()
{
    array<data_type[N][N]> a;
    array<data_type[N][N]> b;
    array<data_type[N][N]> c;

    std::cout << "A:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_static_instance<Order, stencil_impl::pure, array<data_type[N][N]>, N, N>(a, b, c);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_static_instance<Order, stencil_impl::map, array<data_type[N][N]>, N, N>(a, b, c);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_static_instance<Order, stencil_impl::map_reduce, array<data_type[N][N]>, N, N>(a, b, c);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t Order, size_t N>
void test_stencil_dyn()
{
    using array_type = dynarray<data_type, 2>;
    array_type a(N, N);
    array_type b(N, N);
    array_type c(N, N);

    std::cout << "D:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_dyn_instance<Order, stencil_impl::pure, array_type>(a, b, c, N, N);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_dyn_instance<Order, stencil_impl::map, array_type>(a, b, c, N, N);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_dyn_instance<Order, stencil_impl::map_reduce, array_type>(a, b, c, N, N);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t Order, size_t N>
void test_stencil_boost()
{
    typedef boost::multi_array<data_type, 2> array_type;

    array_type a(boost::extents[N][N]);
    array_type b(boost::extents[N][N]);
    array_type c(boost::extents[N][N]);

    std::cout << "B:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_dyn_instance<Order, stencil_impl::pure, array_type>(a, b, c, N, N);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_dyn_instance<Order, stencil_impl::map, array_type>(a, b, c, N, N);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_stencil_dyn_instance<Order, stencil_impl::map_reduce, array_type>(a, b, c, N, N);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t Order, size_t N>
void test_instance()
{
    test_stencil_static<Order, N>();
    test_stencil<Order, N>();
    test_stencil_dyn<Order, N>();
#if 0
    test_stencil_boost<Order, N>();
#endif
}

void test_stencil()
{
    test_instance<1, 100>();
    test_instance<1, 200>();
    test_instance<1, 400>();
    test_instance<1, 800>();
    test_instance<1, 1600>();
    test_instance<1, 3200>();

    test_instance<2, 100>();
    test_instance<2, 200>();
    test_instance<2, 400>();
    test_instance<2, 800>();
    test_instance<2, 1600>();
    test_instance<2, 3200>();

    test_instance<4, 100>();
    test_instance<4, 200>();
    test_instance<4, 400>();
    test_instance<4, 800>();
    test_instance<4, 1600>();
    test_instance<4, 3200>();

    test_instance<8, 100>();
    test_instance<8, 200>();
    test_instance<8, 400>();
    test_instance<8, 800>();
    test_instance<8, 1600>();
    test_instance<8, 3200>();
}

int main(int argc, char *argv[])
{
    test_stencil();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
