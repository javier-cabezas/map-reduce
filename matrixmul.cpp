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
    my_time_point start, end;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            b[i][j] = N * i + j + 1;
        },
        make_range(N, M));

    fill_cache();

    start = my_clock::now();

    if (Impl == matrixmul_impl::pure) {
        for (unsigned i = 0; i < N; ++i) {
            for (unsigned j = 0; j < M; ++j) {
                data_type tmp = 0;
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
                data_type tmp = 0;
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
                                 reduce_ops<data_type>::add,
                                 make_range(N));
            },
            make_range(N, M));
    }

    end = my_clock::now();

    return microsecond_cast(end - start).count();
}

template <matrixmul_impl Impl, typename T>
size_t test_matrixmul_dyn_instance(T &c, T &a, T &b, size_t N, size_t M)
{
    my_time_point start, end;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            b[i][j] = N * i + j + 1;
        },
        make_range(N, M));

    fill_cache();

    start = my_clock::now();

    if (Impl == matrixmul_impl::pure) {
        for (unsigned i = 0; i < N; ++i) {
            for (unsigned j = 0; j < M; ++j) {
                data_type tmp = 0;
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
                data_type tmp = 0;
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
                                 reduce_ops<data_type>::add,
                                 make_range(N));
            },
            make_range(N, M));
    }

    end = my_clock::now();

    return microsecond_cast(end - start).count();
}

template <size_t N>
void test_matrixmul_static()
{
    data_type (&a)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];
    data_type (&b)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];
    data_type (&c)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];

    std::cout << "S:" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_static_instance<matrixmul_impl::pure, data_type (&)[N][N], N, N>(c, a, b);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_static_instance<matrixmul_impl::map, data_type (&)[N][N], N, N>(c, a, b);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_static_instance<matrixmul_impl::map_reduce, data_type (&)[N][N], N, N>(c, a, b);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;
}

template <size_t N>
void test_matrixmul()
{
    array<data_type[N][N]> a;
    array<data_type[N][N]> b;
    array<data_type[N][N]> c;

    std::cout << "A:" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_static_instance<matrixmul_impl::pure, array<data_type[N][N]>, N, N>(c, a, b);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_static_instance<matrixmul_impl::map, array<data_type[N][N]>, N, N>(c, a, b);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_static_instance<matrixmul_impl::map_reduce, array<data_type[N][N]>, N, N>(c, a, b);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t N>
void test_matrixmul_dyn()
{
    using array_type = dynarray<data_type, 2>;
    array_type a(N, N);
    array_type b(N, N);
    array_type c(N, N);

    std::cout << "D:" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_dyn_instance<matrixmul_impl::pure, array_type>(c, a, b, N, N);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_dyn_instance<matrixmul_impl::map, array_type>(c, a, b, N, N);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_dyn_instance<matrixmul_impl::map_reduce, array_type>(c, a, b, N, N);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t N>
void test_matrixmul_boost()
{
    typedef boost::multi_array<data_type, 2> array_type;

    array_type a(boost::extents[N][N]);
    array_type b(boost::extents[N][N]);
    array_type c(boost::extents[N][N]);

    std::cout << "B:" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_dyn_instance<matrixmul_impl::pure, array_type>(c, a, b, N, N);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_dyn_instance<matrixmul_impl::map, array_type>(c, a, b, N, N);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_matrixmul_dyn_instance<matrixmul_impl::map_reduce, array_type>(c, a, b, N, N);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t N>
void test_instance()
{
    test_matrixmul_static<N>();
    test_matrixmul<N>();
    test_matrixmul_dyn<N>();
#if 0
    test_matrixmul_boost<N>();
#endif
}

void test_matrixmul()
{
    test_instance<100>();
    test_instance<200>();
    test_instance<300>();
    test_instance<400>();
    test_instance<500>();
    test_instance<600>();
    test_instance<700>();
    test_instance<800>();
}

int main(int argc, char *argv[])
{
    test_matrixmul();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
