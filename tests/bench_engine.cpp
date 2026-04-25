#include "lotus_core/constants.h"
#include "lotus_core/engine.h"
#include "lotus_core/unicode.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

using namespace lotus_core;

/**
 * @struct LatencyStats
 * @brief Container for performance metrics collected during a benchmark scenario.
 */
struct LatencyStats {
    std::string name;  ///< Descriptive name of the scenario
    double avg;        ///< Average latency in milliseconds
    double max;        ///< Maximum observed latency
    double p99;        ///< 99th percentile latency
    size_t count;      ///< Total number of keystrokes processed
};

/**
 * @class BenchmarkSuite
 * @brief Orchestrates and executes a series of performance benchmark scenarios.
 */
class BenchmarkSuite {
   public:
    explicit BenchmarkSuite(std::string name) : suite_name(std::move(name)) {}

    /**
     * @brief Registers a new benchmark scenario.
     * @param name Descriptive label for the test.
     * @param keys The raw key sequence to be processed by the engine.
     * @param config Optional callback to initialize engine state/options.
     */
    void add_scenario(
        std::string name, std::string keys, std::function<void(Engine&)> config = [](Engine&) {}) {
        scenarios.push_back({std::move(name), std::move(keys), std::move(config)});
    }

    /**
     * @brief Executes all registered scenarios and outputs results to stdout and CSV.
     */
    void run_all() {
        std::cout << "\n[Lotus Core Benchmark] " << suite_name << "\n";
        std::cout << std::string(90, '=') << "\n";
        std::cout << std::left << std::setw(35) << "Scenario" << std::right << std::setw(12)
                  << "Avg (ms)" << std::setw(12) << "Max (ms)" << std::setw(12) << "P99 (ms)"
                  << std::setw(10) << "Keys" << "\n";
        std::cout << std::string(90, '-') << "\n";

        std::vector<double> all_latencies;

        for (const auto& s : scenarios) {
            Engine engine;
            s.config(engine);

            std::vector<double> latencies;
            latencies.reserve(s.keys.length());

            for (char c : s.keys) {
                char32_t key = static_cast<unsigned char>(c);
                if (c == '\b')
                    key = constants::KEY_BACKSPACE;  // ASCII Backspace

                auto start = std::chrono::high_resolution_clock::now();
                engine.process_key(key, {});
                auto end = std::chrono::high_resolution_clock::now();

                std::chrono::duration<double, std::milli> diff = end - start;
                latencies.push_back(diff.count());
                all_latencies.push_back(diff.count());
            }

            auto stats = calculate_stats(s.name, latencies);
            print_stats(stats);
            results.push_back(stats);
        }

        auto overall = calculate_stats("OVERALL TOTAL", all_latencies);
        std::cout << std::string(90, '-') << "\n";
        print_stats(overall);
        std::cout << std::string(90, '=') << "\n\n";

        if (overall.avg > 1.0) {
            std::cerr << "[!] WARNING: Performance target (< 1ms) not met! Avg: " << overall.avg
                      << " ms\n";
        } else {
            std::cout << "[+] SUCCESS: Latency within acceptable bounds.\n";
        }

        save_to_csv();
    }

   private:
    struct Scenario {
        std::string name;
        std::string keys;
        std::function<void(Engine&)> config;
    };

    std::string suite_name;
    std::vector<Scenario> scenarios;
    std::vector<LatencyStats> results;

    LatencyStats calculate_stats(const std::string& name, std::vector<double>& latencies) {
        if (latencies.empty())
            return {name, 0, 0, 0, 0};

        double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
        double avg = sum / static_cast<double>(latencies.size());

        std::sort(latencies.begin(), latencies.end());
        double max = latencies.back();
        double p99 = latencies[static_cast<size_t>(static_cast<double>(latencies.size()) * 0.99)];

        return {name, avg, max, p99, latencies.size()};
    }

    void print_stats(const LatencyStats& s) {
        std::cout << std::left << std::setw(35) << s.name << std::right << std::fixed
                  << std::setprecision(4) << std::setw(12) << s.avg << std::setw(12) << s.max
                  << std::setw(12) << s.p99 << std::setw(10) << s.count << "\n";
    }

    void save_to_csv() {
        const std::string filename = "benchmark_results.csv";
        std::ofstream csv(filename);
        csv << "Scenario,AvgMS,MaxMS,P99MS,Keys\n";
        for (const auto& r : results) {
            csv << r.name << "," << r.avg << "," << r.max << "," << r.p99 << "," << r.count << "\n";
        }
        std::cout << "[i] Full report written to " << filename << "\n";
    }
};

/**
 * @brief Register Standard Input Method scenarios.
 */
void register_standard_scenarios(BenchmarkSuite& suite) {
    // Standard Telex: Formal writing with non-flexible markers.
    suite.add_scenario("IM: Telex (Formal)",
                       "lotus engine laf mootj bbooj goox tieengs vieetj hieenj ddaaij, "
                       "toois ưu hoas cho hieeuu nng dda rrat cao.",
                       [](Engine& e) { e.set_method(InputMethod::TELEX); });

    // Standard VNI: Using number keys for markers.
    suite.add_scenario("IM: VNI (Formal)",
                       "lotus engine la2 mo^6t5 bo^65 go~4 tie^6ng1 vie^6t5 hie^6n5 dda9i5, "
                       "to^6i1 u*7u hoas1 cho hie^6u65 na8ng dda~4 ra^6t1 cao.",
                       [](Engine& e) { e.set_method(InputMethod::VNI); });
}

/**
 * @brief Register Advanced Engine Feature scenarios.
 */
void register_advanced_scenarios(BenchmarkSuite& suite) {
    // Flexible Telex: Markers can be placed anywhere in the syllable.
    suite.add_scenario("Feature: Flexible Telex",
                       "viecje goox tieengs vieetj chuwa bao gioof deeex daangf ddeens thees. "
                       "dosd ddaay laf mootj vis duwj veea viecje goox nhanh.",
                       [](Engine& e) {
                           e.set_method(InputMethod::TELEX);
                           e.set_auto_restore(true);
                       });

    // English Restoration: Automatic detection of English words to prevent false transformations.
    suite.add_scenario("Feature: English Restoration",
                       "Trong lapj trình C++, vieecj manage memory laf rrat' quan trongj. "
                       "Sử dụng smart pointers giúp avoid memory leaks.",
                       [](Engine& e) {
                           e.set_method(InputMethod::TELEX);
                           e.set_auto_restore(true);
                       });

    // Macros/Shortcuts: Expanding short triggers into full strings.
    suite.add_scenario("Feature: Macros/Shortcuts", "vn hcm tks bst ", [](Engine& e) {
        e.set_method(InputMethod::TELEX);
        e.add_shortcut("vn", "Việt Nam");
        e.add_shortcut("hcm", "Thành phố Hồ Chí Minh");
        e.add_shortcut("tks", "Cảm ơn");
    });
}

/**
 * @brief Register Workflow and Stress scenarios.
 */
void register_stress_scenarios(BenchmarkSuite& suite) {
    // Interactive Editing: Simulating user backspacing and correcting text.
    suite.add_scenario("Workflow: Editing/Correction",
                       "xin chaof\b\b\b\b\boof mootj\b\b\b\b\btj truowngf\b\b\b\b\b\b\b\bngf ",
                       [](Engine& e) { e.set_method(InputMethod::TELEX); });

    // Customized Session: Worst-case complexity with all features toggled on.
    suite.add_scenario("Workflow: Full Customization",
                       "wow mootj buoori sangs ddepj troi`. Goox tieengs Vieetj thajt vui. "
                       "anh dda~ dduwowjc ddawng kys.",
                       [](Engine& e) {
                           e.set_method(InputMethod::TELEX);
                           e.set_free_w(FreeWOption::ALWAYS);
                           e.set_std_uo(true);
                           e.set_double_space_to_period(true);
                           e.set_auto_capitalize(true);
                       });

    // Stress Test: Repeated processing of the longest possible Vietnamese syllables.
    std::string stress = "nghieengs dduowngf tuyeens quyeet' nghieenj huyeexnh ";
    for (int i = 0; i < 5; ++i)
        stress += stress;
    suite.add_scenario("Stress: Complex Syllables", stress,
                       [](Engine& e) { e.set_method(InputMethod::TELEX); });
}

int main() {
    BenchmarkSuite suite("Core Engine Latency Analysis");

    register_standard_scenarios(suite);
    register_advanced_scenarios(suite);
    register_stress_scenarios(suite);

    suite.run_all();

    return 0;
}
