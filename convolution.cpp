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

enum class convolution_impl {
    pure,
    map,
    map_reduce
};

template <int Order, convolution_impl Impl, typename T1, typename T2, size_t N, size_t M, bool Test = DoTest>
size_t test_convolution_static_instance(T1 &a, T1 &b, T1 &c, T2 &conv)
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

    map([&](int i, int j)
        {
            conv[i][j] = i + j;
        },
        make_range(2 * Order + 1,
                   2 * Order + 1));

    fill_cache();

    start = my_clock::now();

    if (Impl == convolution_impl::pure || Test) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                data_type tmp = a[i][j];
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
                data_type tmp = a[i][j];
                for (int k1 = 0; k1 < 2 * Order + 1; ++k1) {
                    for (int k2 = 0; k2 < 2 * Order + 1; ++k2) {
                        tmp += conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                    }
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
    }

    if (Test) {
        //assert(b == c);
    }

    if (Impl == convolution_impl::map_reduce) {
        map([&](int i, int j)
            {
                b[i][j] = reduce([&](int k1, int k2)
                                 {
                                      return conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                                 },
                                 reduce_ops<data_type>::add,
                                 make_range(dim<int>(2 * Order + 1),
                                            dim<int>(2 * Order + 1)));
            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
    }

    if (Test) {
        //assert(b == c);
    }

    end = my_clock::now();

    return microsecond_cast(end - start).count();
}

template <int Order, convolution_impl Impl, typename T, bool Test = DoTest>
size_t test_convolution_dyn_instance(T &a, T &b, T &c, T &conv, size_t N, size_t M)
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

    map([&](int i, int j)
        {
            conv[i][j] = i + j;
        },
        make_range(2 * Order + 1,
                   2 * Order + 1));

    fill_cache();

    start = my_clock::now();

    if (Impl == convolution_impl::pure || Test) {
        for (unsigned i = Order; i < N - Order; ++i) {
            for (unsigned j = Order; j < M - Order; ++j) {
                data_type tmp = 0.f;
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
                data_type tmp = 0.f;
                for (int k1 = 0; k1 < 2 * Order + 1; ++k1) {
                    for (int k2 = 0; k2 < 2 * Order + 1; ++k2) {
                        tmp += conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                    }
                }

                b[i][j] = tmp;
            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
    }

    if (Test) {
        assert(b == c);
    }

    if (Impl == convolution_impl::map_reduce || Test) {
        map([&](int i, int j)
            {
                b[i][j] = reduce([&](int k1, int k2)
                                 {
                                      return conv[k1][k2] * a[i - Order + k1][j - Order + k2];
                                 },
                                 reduce_ops<data_type>::add,
                                 make_range(dim<int>(2 * Order + 1),
                                            dim<int>(2 * Order + 1)));

            },
            make_range(dim<int>(Order, int(N) - Order),
                       dim<int>(Order, int(M) - Order)));
    }

    if (Test) {
        map([&](int i, int j)
            {
                if (b[i][j] != c[i][j]) {
                    printf("(%d, %d) Expected %f, got %f\n", i, j, b[i][j], c[i][j]);
                }
            },
            make_range(N, M));
        //assert(b == c);
    }

    end = my_clock::now();

    return microsecond_cast(end - start).count();
}

template <size_t Order, size_t N>
void test_convolution_static()
{
    data_type (&a)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];
    data_type (&b)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];
    data_type (&c)[N][N] = *(data_type (*)[N][N]) new data_type[N * N];

    data_type (&conv)[2 * Order + 1][2 * Order + 1] = *(data_type (*)[2 * Order + 1][2 * Order + 1]) new data_type[(2 * Order + 1) * (2 * Order + 1)];

    std::cout << "S:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_static_instance<Order,
                                                  convolution_impl::pure,
                                                  data_type (&)[N][N],
                                                  data_type (&)[2 * Order + 1][2 * Order + 1],
                                                  N, N>(a, b, c, conv);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_static_instance<Order,
                                                  convolution_impl::map,
                                                  data_type (&)[N][N],
                                                  data_type (&)[2 * Order + 1][2 * Order + 1],
                                                  N, N>(a, b, c, conv);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_static_instance<Order,
                                                  convolution_impl::map_reduce,
                                                  data_type (&)[N][N],
                                                  data_type (&)[2 * Order + 1][2 * Order + 1],
                                                  N, N>(a, b, c, conv);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;

    delete [] &conv;
}

template <size_t Order, size_t N>
void test_convolution()
{
    array<data_type[N][N]> a;
    array<data_type[N][N]> b;
    array<data_type[N][N]> c;

    array<data_type[2 * Order + 1][2 * Order + 1]> conv;

    std::cout << "A:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_static_instance<Order,
                                                  convolution_impl::pure,
                                                  array<data_type[N][N]>,
                                                  array<data_type[2 * Order + 1][2 * Order + 1]>,
                                                  N, N>(a, b, c, conv);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_static_instance<Order,
                                                  convolution_impl::map,
                                                  array<data_type[N][N]>,
                                                  array<data_type[2 * Order + 1][2 * Order + 1]>,
                                                  N, N>(a, b, c, conv);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_static_instance<Order,
                                                  convolution_impl::map_reduce,
                                                  array<data_type[N][N]>,
                                                  array<data_type[2 * Order + 1][2 * Order + 1]>,
                                                  N, N>(a, b, c, conv);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t Order, size_t N>
void test_convolution_dyn()
{
    using array_type = dynarray<data_type, 2>;
    array_type a(N, N);
    array_type b(N, N);
    array_type c(N, N);

    array_type conv(2 * Order + 1, 2 * Order + 1);

    std::cout << "D:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_dyn_instance<Order,
                                               convolution_impl::pure,
                                               array_type>(a, b, c, conv, N, N);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_dyn_instance<Order,
                                               convolution_impl::map,
                                               array_type>(a, b, c, conv, N, N);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_dyn_instance<Order,
                                               convolution_impl::map_reduce,
                                               array_type>(a, b, c, conv, N, N);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t Order, size_t N>
void test_convolution_boost()
{
    typedef boost::multi_array<data_type, 2> array_type;

    array_type a(boost::extents[N][N]);
    array_type b(boost::extents[N][N]);
    array_type c(boost::extents[N][N]);

    array_type conv(boost::extents[2 * Order + 1][2 * Order + 1]);

    std::cout << "B:" << Order << "_" << N << ",";

    std::vector<size_t> usecs(Iterations);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_dyn_instance<Order,
                                                  convolution_impl::pure,
                                                  array_type>(a, b, c, conv, N, N);
    }
    print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_dyn_instance<Order,
                                                  convolution_impl::map,
                                                  array_type>(a, b, c, conv, N, N);
    }
    std::cout << ","; print_stats(usecs);

    for (unsigned it = 0; it < Iterations; ++it) {
        usecs[it] = test_convolution_dyn_instance<Order,
                                                  convolution_impl::map_reduce,
                                                  array_type>(a, b, c, conv, N, N);
    }
    std::cout << ","; print_stats(usecs);

    std::cout << std::endl;
}

template <size_t Order, size_t N>
void test_instance()
{
    test_convolution_static<Order, N>();
    test_convolution<Order, N>();
    test_convolution_dyn<Order, N>();
#if 0
    test_convolution_boost<Order, N>();
#endif
}

void test_convolution()
{
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
    test_convolution();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
