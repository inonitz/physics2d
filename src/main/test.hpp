#pragma once
#include "../base.hpp"
#include <random>
#include <chrono>
#include <ratio>
#include <string>




int testing();


#define BENCHMARK_STATUS_FINISHED    (0b000)
#define BENCHMARK_STATUS_TBD         (0b001)
#define BENCHMARK_STATUS_SUCCESS     (0b010)
#define BENCHMARK_STATUS_FAIL        (0b100)


struct BenchmarkResult
{


    using chrono_ns = std::chrono::nanoseconds;
    std::array<char, 200> name;
    f32                   measured_ns;
    u8                    status;
};


struct Benchmark {
    std::string_view title;
    std::unique_ptr<std::vector<BenchmarkResult>> data;
};


template <class TimeUnit = std::chrono::nanoseconds, class ClockType = std::chrono::steady_clock> class Timer {
    using TimePoint = typename ClockType::time_point;
    
	TimePoint m_start, m_end;

public:
    void tick() { 
        m_end = TimePoint{}; 
        m_start = ClockType::now(); 
    }
    
    void tock() { m_end = ClockType::now(); }
    
    template <class T = TimeUnit> auto duration() const {  
        return std::chrono::duration_cast<T>(m_end - m_start); 
    }

    template<class DataType> auto duration_cast(TimeUnit const& tu) const {
        return std::chrono::duration_cast<std::chrono::duration<DataType, std::nano>>(tu);
    }
};


template<class TimeUnit = std::chrono::milliseconds, class ClockType = std::chrono::steady_clock> struct measure
{
    template<class Func, class ...Args> static auto duration(Func&& func, Args&&... args)
    {
        auto start = ClockType::now();
        std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
        return std::chrono::duration_cast<TimeUnit>(ClockType::now() - start);
    }
};


template<class Generator = std::mt19937>
struct NumberGenerator
{
    using intDistResultType = std::uniform_int_distribution<int32_t>::result_type;
    using uintDistResultType = std::uniform_int_distribution<uint32_t>::result_type;
    using fDistResultType = std::uniform_real_distribution<f32>::result_type;
    Generator                               generator;
    std::uniform_int_distribution<uint32_t> uintdist;
    std::uniform_int_distribution<int32_t>  intdist;
    std::uniform_real_distribution<f32>     floatdist;


    NumberGenerator(size_t initialSeed = 0)
    {
        initialSeed += (initialSeed == 0) * std::random_device()(); 
        generator.seed(initialSeed);
        return;
    }


    intDistResultType  randu() { return uintdist(generator);  }
    uintDistResultType randi() { return intdist(generator);   }
    fDistResultType    randf() { return floatdist(generator); }

};