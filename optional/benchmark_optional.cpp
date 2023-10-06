#include <benchmark/benchmark.h>
#include <filesystem>
#include <optional>

bool GetPath(const std::string&, std::filesystem::path& path);
std::optional<std::filesystem::path> GetPath(const std::string&);

static void ReturnByReference(benchmark::State& state)
{
	std::string str("..");
	int counter = 0;
	std::filesystem::path path;
	for (auto _ : state) {
		if (GetPath(str, path)) {
			counter++;
		}
		benchmark::DoNotOptimize(path);
	}
	benchmark::DoNotOptimize(counter);
}

static void ReturnByValue(benchmark::State& state)
{
	std::string str("..");

	int counter = 0;
	for (auto _ : state) {
		auto result = GetPath(str);
		if (result) {
			counter++;
		}
		benchmark::DoNotOptimize(result);
	}
	benchmark::DoNotOptimize(counter);
}
BENCHMARK(ReturnByReference);
BENCHMARK(ReturnByValue);
BENCHMARK_MAIN();
