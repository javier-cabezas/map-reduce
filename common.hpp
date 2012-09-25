#ifndef COMMON_HPP
#define COMMON_HPP

#include <chrono>

#ifdef DEBUG
using data_type = long int;

static const bool DoTest = true;
static const unsigned Iterations = 1;
#else
using data_type = float;

static const bool DoTest = false;
static const unsigned Iterations = 10;
#endif

using my_clock = std::chrono::steady_clock;
using my_time_point = std::chrono::time_point<my_clock>;

#define microsecond_cast std::chrono::duration_cast<std::chrono::microseconds>

static
inline
void print_banner(const std::string &name)
{
    std::cout << std::string(name.size(), '=') << std::endl;
    std::cout << name << std::endl;
    std::cout << std::string(name.size(), '=') << std::endl;
}

static
void fill_cache()
{
    static int trash[1024 * 1024 * 16];

    for (int &t : trash) {
        t++;
    }
}

template <typename T>
void
print_stats(std::vector<T> &timesv)
{
    double min = *std::min_element(timesv.begin(), timesv.end());
    //auto max = *std::max_element(timesv.begin(), timesv.end());
    double mean = std::accumulate(timesv.begin(), timesv.end(), 0.0) / double(timesv.size());

    std::vector<double> zero_mean(timesv.size());
    for (unsigned it = 0; it < timesv.size(); ++it) {
        zero_mean[it] = timesv[it];
    }

    std::transform(zero_mean.begin(), zero_mean.end(), zero_mean.begin(),
                   [&mean](const double &d) -> double
                   {
                       return std::pow(d - mean, 2);
                   });
   //auto stdd = std::sqrt(std::accumulate(timesv.begin(), timesv.end(), 0.0) / double(timesv.size()));

   std::cout << std::fixed << min;
#if 0
   std::cout << "min,max,mean,stdd" << std::endl;
   std::cout << min << "," << max << "," << std::fixed << mean << "," << stdd << std::endl;
#endif
}

#endif
