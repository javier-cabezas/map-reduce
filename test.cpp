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

template <typename T>
void test_matrixmul_dyn_gold(dynarray<T, 2> &c, 
                             const dynarray<T, 2> &a, 
                             const dynarray<T, 2> &b)
{
    for (unsigned i = 0; i < a.get_size(0); ++i) {
        for (unsigned j = 0; j < a.get_size(1); ++j) {
            T tmp = 0;
            for (unsigned k = 0; k < a.get_size(0); ++k) {
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
int test_matrixmul_static_instance(T &c, T &a, T &b)
{
    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            b[i][j] = N * i + j + 1;
        },
        make_range(N, M));

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

    return 0;
}

template <matrixmul_impl Impl, typename T>
int test_matrixmul_dyn_instance(T &c, T &a, T &b, size_t N, size_t M)
{ 
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

    return 0;
}

enum class stencil_impl {
    pure,
    map,
    map_reduce
};

template <stencil_impl Impl, typename T, size_t N, size_t M, bool Test = false>
int test_stencil_static_instance(T &a, T &b, T &c)
{
    const int order = 8;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            if (Test) {
                b[i][j] = 0;
                c[i][j] = 0;
            }
        },
        make_range(N, M));

    if (Impl == stencil_impl::pure) {
        for (unsigned i = order; i < N - order; ++i) {
            for (unsigned j = order; j < M - order; ++j) {
                long tmp = a[i][j];
                for (int k = 1; k <= order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                c[i][j] = tmp;
            }
        }
    }

    if (Impl == stencil_impl::map) {
        map([&](int i, int j)
            {
                long tmp = a[i][j];
                for (int k = 1; k <= order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                b[i][j] = tmp;
            },
            make_range(dim_type<int>({ order, int(N) - order}),
                       dim_type<int>({ order, int(M) - order})));
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
                                           make_range(dim_type<int>({1, order + 1})));

            },
            make_range(dim_type<int>({ order, int(N) - order}),
                       dim_type<int>({ order, int(M) - order})));
    }

    return 0;
}

template <stencil_impl Impl, typename T, bool Test = false>
int test_stencil_dyn_instance(T &a, T &b, T &c, size_t N, size_t M)
{
    const int order = 8;

    map([&](int i, int j)
        {
            a[i][j] = N * i + j + 1;
            if (Test) {
                b[i][j] = 0;
                c[i][j] = 0;
            }
        },
        make_range(N, M));

    if (Impl == stencil_impl::pure || Test) {
        for (unsigned i = order; i < N - order; ++i) {
            for (unsigned j = order; j < M - order; ++j) {
                long tmp = a[i][j];
                for (int k = 1; k <= order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                c[i][j] = tmp;
            }
        }
    }

    if (Impl == stencil_impl::map || Test) {
        map([&](int i, int j)
            {
                long tmp = a[i][j];
                for (int k = 1; k <= order; ++k) {
                    tmp += a[i - k][j] + a[i + k][j] +
                           a[i][j - k] + a[i][j + k];
                }

                b[i][j] = tmp;
            },
            make_range(dim_type<int>({ order, int(N) - order}),
                       dim_type<int>({ order, int(M) - order})));
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
                                           make_range(dim_type<int>({1, order + 1})));

            },
            make_range(dim_type<int>({ order, int(N) - order}),
                       dim_type<int>({ order, int(M) - order})));
    }

    if (Test) {
        assert(b == c);
    }

    return 0;
}

int test_matrixmul_static()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "=================================" << std::endl;
    std::cout << "Matrix multiplication pure static" << std::endl;
    std::cout << "=================================" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    long (&a)[N][M] = *(long (*)[N][M]) new long[N * M];
    long (&b)[N][M] = *(long (*)[N][M]) new long[N * M];
    long (&c)[N][M] = *(long (*)[N][M]) new long[N * M];

    start = std::chrono::system_clock::now();
    test_matrixmul_static_instance<matrixmul_impl::pure, long (&)[N][M], N, M>(c, a, b);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_static_instance<matrixmul_impl::map, long (&)[N][M], N, M>(c, a, b);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_static_instance<matrixmul_impl::map_reduce, long (&)[N][M], N, M>(c, a, b);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;

    return 0;
}

int test_stencil_static()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "==========================" << std::endl;
    std::cout << "Matrix stencil pure static" << std::endl;
    std::cout << "==========================" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    long (&a)[N][M] = *(long (*)[N][M]) new long[N * M];
    long (&b)[N][M] = *(long (*)[N][M]) new long[N * M];
    long (&c)[N][M] = *(long (*)[N][M]) new long[N * M];

    start = std::chrono::system_clock::now();
    test_stencil_static_instance<stencil_impl::pure, long (&)[N][M], N, M>(a, b, c);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_static_instance<stencil_impl::map, long (&)[N][M], N, M>(a, b, c);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_static_instance<stencil_impl::map_reduce, long (&)[N][M], N, M>(a, b, c);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    delete [] &a;
    delete [] &b;
    delete [] &c;

    return 0;
}

int test_matrixmul()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "=====================" << std::endl;
    std::cout << "Matrix multiplication" << std::endl;
    std::cout << "=====================" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    array<long[N][M]> a;
    array<long[N][M]> b;
    array<long[N][M]> c;

    start = std::chrono::system_clock::now();
    test_matrixmul_static_instance<matrixmul_impl::pure, array<long[N][M]>, N, M>(c, a, b);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_static_instance<matrixmul_impl::map, array<long[N][M]>, N, M>(c, a, b);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_static_instance<matrixmul_impl::map_reduce, array<long[N][M]>, N, M>(c, a, b);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;
}

int test_stencil()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "==============" << std::endl;
    std::cout << "Matrix stencil" << std::endl;
    std::cout << "==============" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    array<long[N][M]> a;
    array<long[N][M]> b;
    array<long[N][M]> c;

    start = std::chrono::system_clock::now();
    test_stencil_static_instance<stencil_impl::pure, array<long[N][M]>, N, M>(a, b, c);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_static_instance<stencil_impl::map, array<long[N][M]>, N, M>(a, b, c);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_static_instance<stencil_impl::map_reduce, array<long[N][M]>, N, M>(a, b, c);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    return 0;
}

int test_matrixmul_dyn()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "================================" << std::endl;
    std::cout << "Matrix multiplication (dynarray)" << std::endl;
    std::cout << "================================" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    using array_type = dynarray<long, 2>;
    array_type a(N, M);
    array_type b(N, M);

    array_type c(N, M), c_gold(N, M);

    start = std::chrono::system_clock::now();
    test_matrixmul_dyn_instance<matrixmul_impl::pure, array_type>(c, a, b, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_dyn_instance<matrixmul_impl::map, array_type>(c, a, b, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_dyn_instance<matrixmul_impl::map_reduce, array_type>(c, a, b, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;
}

int test_stencil_dyn()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "=========================" << std::endl;
    std::cout << "Matrix stencil (dynarray)" << std::endl;
    std::cout << "=========================" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    using array_type = dynarray<long, 2>;
    array_type a(N, M);
    array_type b(N, M);

    array_type c(N, M), c_gold(N, M);

    start = std::chrono::system_clock::now();
    test_stencil_dyn_instance<stencil_impl::pure, array_type>(a, b, c, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_dyn_instance<stencil_impl::map, array_type>(a, b, c, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_dyn_instance<stencil_impl::map_reduce, array_type>(a, b, c, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    return 0;
}

int test_matrixmul_boost()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "=============================" << std::endl;
    std::cout << "Matrix multiplication (boost)" << std::endl;
    std::cout << "=============================" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    typedef boost::multi_array<long, 2> array_type;

    array_type a(boost::extents[N][M]);
    array_type b(boost::extents[N][M]);

    array_type c(boost::extents[N][M]);

    start = std::chrono::system_clock::now();
    test_matrixmul_dyn_instance<matrixmul_impl::pure, array_type>(c, a, b, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_dyn_instance<matrixmul_impl::map, array_type>(c, a, b, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_matrixmul_dyn_instance<matrixmul_impl::map_reduce, array_type>(c, a, b, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;
}

int test_stencil_boost()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;

    std::cout << "======================" << std::endl;
    std::cout << "Matrix stencil (boost)" << std::endl;
    std::cout << "======================" << std::endl;

    static const size_t N = 1000;
    static const size_t M = 1000;

    typedef boost::multi_array<long, 2> array_type;

    array_type a(boost::extents[N][M]);
    array_type b(boost::extents[N][M]);

    array_type c(boost::extents[N][M]);

    start = std::chrono::system_clock::now();
    test_stencil_dyn_instance<stencil_impl::pure, array_type>(a, b, c, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_dyn_instance<stencil_impl::map, array_type>(a, b, c, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    start = std::chrono::system_clock::now();
    test_stencil_dyn_instance<stencil_impl::map_reduce, array_type>(a, b, c, N, M);
    end = std::chrono::system_clock::now();
    std::cout << "1000x1000: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " usecs " << std::endl;

    return 0;
}

int test_array()
{
    array<int[10][10]> a;
    array<int[10][10]> b;

    for (unsigned i = 0; i < a.get_size<0>(); ++i) {
        for (unsigned j = 0; j < a.get_size<1>(); ++j) {
            a[i][j] = i * j;
            b(i, j) = i * j;
        }
    }

    for (unsigned i = 0; i < a.get_size<0>(); ++i) {
        for (unsigned j = 0; j < a.get_size<1>(); ++j) {
            assert(b[i][j] == a(i, j));
        }
    }

    return 0;
}

int test_dynarray()
{
    dynarray<int, 2> a(10, 10);
    dynarray<int, 2> b(10, 10);

    for (unsigned i = 0; i < a.get_size(0); ++i) {
        for (unsigned j = 0; j < a.get_size(1); ++j) {
            a[i][j] = i * 10 + j;
            b(i, j) = i * 10 + j;
        }
    }

    for (unsigned i = 0; i < a.get_size(0); ++i) {
        for (unsigned j = 0; j < a.get_size(1); ++j) {
            assert(b[i][j] == a(i, j));
        }
    }

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
    test_array();
    test_dynarray();
    test_ref();

    test_reduction();

    test_matrixmul_static();
    test_matrixmul();
    test_matrixmul_dyn();
    test_matrixmul_boost();

    test_stencil_static();
    test_stencil();
    test_stencil_dyn();
    test_stencil_boost();

    return 0;
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
