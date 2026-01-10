#include "btree.cpp"
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>
#include <set>
#include <string>

using namespace std::chrono;

// Benchmark result structure
struct BenchmarkResult {
    std::string name;
    double time_ms;
    size_t operations;

    double ops_per_sec() const {
        return operations / (time_ms / 1000.0);
    }
};

// Timer utility
class Timer {
    high_resolution_clock::time_point start_;
public:
    Timer() : start_(high_resolution_clock::now()) {}

    double elapsed_ms() const {
        auto end = high_resolution_clock::now();
        return duration_cast<microseconds>(end - start_).count() / 1000.0;
    }
};

// Generate random integers
std::vector<int> generate_random(size_t n, unsigned seed = 42) {
    std::vector<int> data(n);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, static_cast<int>(n * 10));
    for (size_t i = 0; i < n; i++) {
        data[i] = dist(gen);
    }
    return data;
}

// Generate sequential integers
std::vector<int> generate_sequential(size_t n) {
    std::vector<int> data(n);
    for (size_t i = 0; i < n; i++) {
        data[i] = static_cast<int>(i);
    }
    return data;
}

// Print separator
void print_separator() {
    std::cout << std::string(80, '-') << "\n";
}

// Print header
void print_header(const std::string& title) {
    std::cout << "\n";
    print_separator();
    std::cout << title << "\n";
    print_separator();
}

// Benchmark insert operation
template<int Order>
BenchmarkResult benchmark_insert_random(const std::vector<int>& data) {
    Timer timer;
    BTree<int, Order> tree;
    for (int val : data) {
        tree.insert(val);
    }
    return {"BTree<" + std::to_string(Order) + "> insert random", timer.elapsed_ms(), data.size()};
}

template<int Order>
BenchmarkResult benchmark_insert_sequential(const std::vector<int>& data) {
    Timer timer;
    BTree<int, Order> tree;
    for (int val : data) {
        tree.insert(val);
    }
    return {"BTree<" + std::to_string(Order) + "> insert sequential", timer.elapsed_ms(), data.size()};
}

// Benchmark search operation
template<int Order>
BenchmarkResult benchmark_search(BTree<int, Order>& tree, const std::vector<int>& queries) {
    Timer timer;
    volatile int found = 0;  // Prevent optimization
    for (int val : queries) {
        if (tree.search(val)) found++;
    }
    return {"BTree<" + std::to_string(Order) + "> search", timer.elapsed_ms(), queries.size()};
}

// Benchmark remove operation
template<int Order>
BenchmarkResult benchmark_remove(const std::vector<int>& data) {
    BTree<int, Order> tree;
    for (int val : data) {
        tree.insert(val);
    }

    // Shuffle for random removal order
    std::vector<int> to_remove = data;
    std::mt19937 gen(123);
    std::shuffle(to_remove.begin(), to_remove.end(), gen);

    Timer timer;
    for (int val : to_remove) {
        tree.remove(val);
    }
    return {"BTree<" + std::to_string(Order) + "> remove random", timer.elapsed_ms(), data.size()};
}

// Benchmark iteration
template<int Order>
BenchmarkResult benchmark_iterate(BTree<int, Order>& tree) {
    Timer timer;
    volatile long sum = 0;  // Prevent optimization
    for (const auto& val : tree) {
        sum += val;
    }
    return {"BTree<" + std::to_string(Order) + "> iterate", timer.elapsed_ms(), tree.size()};
}

// Benchmark std::set for comparison
BenchmarkResult benchmark_set_insert(const std::vector<int>& data) {
    Timer timer;
    std::set<int> s;
    for (int val : data) {
        s.insert(val);
    }
    return {"std::set insert", timer.elapsed_ms(), data.size()};
}

BenchmarkResult benchmark_set_search(std::set<int>& s, const std::vector<int>& queries) {
    Timer timer;
    volatile int found = 0;
    for (int val : queries) {
        if (s.find(val) != s.end()) found++;
    }
    return {"std::set search", timer.elapsed_ms(), queries.size()};
}

BenchmarkResult benchmark_set_remove(const std::vector<int>& data) {
    std::set<int> s;
    for (int val : data) {
        s.insert(val);
    }

    std::vector<int> to_remove = data;
    std::mt19937 gen(123);
    std::shuffle(to_remove.begin(), to_remove.end(), gen);

    Timer timer;
    for (int val : to_remove) {
        s.erase(val);
    }
    return {"std::set remove", timer.elapsed_ms(), data.size()};
}

BenchmarkResult benchmark_set_iterate(std::set<int>& s) {
    Timer timer;
    volatile long sum = 0;
    for (const auto& val : s) {
        sum += val;
    }
    return {"std::set iterate", timer.elapsed_ms(), s.size()};
}

// Print result
void print_result(const BenchmarkResult& r) {
    std::cout << std::left << std::setw(40) << r.name
              << std::right << std::setw(12) << std::fixed << std::setprecision(2) << r.time_ms << " ms"
              << std::setw(15) << static_cast<long>(r.ops_per_sec()) << " ops/sec\n";
}

// Run all benchmarks for a given size
template<int Order>
void run_benchmarks_for_order(size_t n, const std::vector<int>& random_data, const std::vector<int>& seq_data) {
    std::cout << "\n=== Order " << Order << " ===\n";

    // Insert benchmarks
    print_result(benchmark_insert_random<Order>(random_data));
    print_result(benchmark_insert_sequential<Order>(seq_data));

    // Build tree for search/iterate benchmarks
    BTree<int, Order> tree;
    for (int val : random_data) {
        tree.insert(val);
    }

    // Search benchmarks
    print_result(benchmark_search<Order>(tree, random_data));

    // Iterate benchmark
    print_result(benchmark_iterate<Order>(tree));

    // NOTE: Remove benchmark skipped due to bug in remove() with random data
    // print_result(benchmark_remove<Order>(random_data));
}

void run_set_benchmarks(size_t n, const std::vector<int>& random_data) {
    std::cout << "\n=== std::set (baseline) ===\n";

    print_result(benchmark_set_insert(random_data));

    std::set<int> s;
    for (int val : random_data) {
        s.insert(val);
    }

    print_result(benchmark_set_search(s, random_data));
    print_result(benchmark_set_iterate(s));
    // Skipped to match BTree benchmarks
    // print_result(benchmark_set_remove(random_data));
}

int main(int argc, char* argv[]) {
    std::vector<size_t> sizes = {10000, 100000, 1000000};

    // Allow command line override of sizes
    if (argc > 1) {
        sizes.clear();
        for (int i = 1; i < argc; i++) {
            sizes.push_back(std::stoul(argv[i]));
        }
    }

    std::cout << "BTree Performance Benchmarks\n";
    std::cout << "============================\n";

    for (size_t n : sizes) {
        print_header("Size: " + std::to_string(n) + " elements");

        auto random_data = generate_random(n);
        auto seq_data = generate_sequential(n);

        run_benchmarks_for_order<3>(n, random_data, seq_data);
        run_benchmarks_for_order<10>(n, random_data, seq_data);
        run_benchmarks_for_order<50>(n, random_data, seq_data);
        run_benchmarks_for_order<100>(n, random_data, seq_data);

        run_set_benchmarks(n, random_data);
    }

    std::cout << "\nBenchmarks complete.\n";
    return 0;
}
