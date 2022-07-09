#include "../shared_var/shared_var.hpp"
#include "../shared_var/multithread.hpp"
#include "../shared_var/atomic_wrapper.hpp"

#include <benchmark/benchmark.h>

#include <atomic>

static void var(benchmark::State& state) {
  std::srand(1);

  double d = 0.0;

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    d += double(rand() % 1000) / 1000;
    benchmark::DoNotOptimize(d);
  }
}
// Register the function as a benchmark
BENCHMARK(var);

static void ptr(benchmark::State& state) {
  std::srand(1);

  double d = 0.0;
  double * p = &d;

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    *p += double(rand() % 1000) / 1000;
    benchmark::DoNotOptimize(d);
  }
}
// Register the function as a benchmark
BENCHMARK(ptr);

static void shared_var(benchmark::State& state) {
  std::srand(1);
  
  shared::map_type<std::string> map;

  auto d = shared::make_var<double>(map, "V0", 0.0);

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    d = d + double(rand() % 1000) / 1000;
    benchmark::DoNotOptimize(d);
  }
}
// Register the function as a benchmark
BENCHMARK(shared_var);

static void shared_get(benchmark::State& state) {
  std::srand(1);
  
  shared::map_type<std::string> map;

  shared::create<double>(map, "V0", 0.0);

  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    shared::auto_get<double>(map, "V0") += double(rand() % 1000) / 1000;
  }
}
// Register the function as a benchmark
BENCHMARK(shared_get);

static void shared_var_atomic(benchmark::State& state) {
  std::srand(1);
  
  shared::thread_safe::ts_var_map_t<std::string> map;

  auto d = shared::atomic::make_atomic_var<double>(map, "V0", 0.0);
  
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    d = d.load() + double(rand() % 1000) / 1000;
    benchmark::DoNotOptimize(d);
  }
}
// Register the function as a benchmark
BENCHMARK(shared_var_atomic);

// Run the benchmark
BENCHMARK_MAIN();
