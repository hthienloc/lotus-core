#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <map>
#include <functional>

using namespace lotus_engine;

/**
 * @brief Statistics for a single benchmark scenario.
 */
struct LatencyStats {
    std::string name;
    double avg;
    double max;
    double p99;
    size_t count;
};

/**
 * @brief Realistic benchmark suite for Lotus Engine.
 * Simulates various typing styles, macros, and user configurations.
 */
class BenchmarkSuite {
public:
    BenchmarkSuite(const std::string& name) : suite_name(name) {}

    /**
     * @brief Adds a scenario to the suite.
     * @param name Name of the scenario.
     * @param keys The raw key sequence to simulate.
     * @param config A lambda to configure the engine state.
     */
    void add_scenario(const std::string& name, const std::string& keys, 
                      std::function<void(Engine&)> config = [](Engine&){}) {
        scenarios.push_back({name, keys, config});
    }

    void run_all() {
        std::cout << "Running Benchmark Suite: " << suite_name << "\n";
        std::cout << std::string(85, '-') << "\n";
        std::cout << std::left << std::setw(30) << "Scenario" 
                  << std::right << std::setw(12) << "Avg (ms)" 
                  << std::setw(12) << "Max (ms)" 
                  << std::setw(12) << "P99 (ms)" 
                  << std::setw(10) << "Keys" << "\n";
        std::cout << std::string(85, '-') << "\n";

        std::vector<double> all_latencies;

        for (const auto& s : scenarios) {
            Engine engine;
            s.config(engine);
            
            std::vector<double> latencies;
            latencies.reserve(s.keys.length());

            for (char c : s.keys) {
                char32_t key = static_cast<unsigned char>(c);
                if (c == '\b') key = 8; // Simulate Backspace

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
        std::cout << std::string(85, '-') << "\n";
        print_stats(overall);
        std::cout << std::string(85, '=') << "\n\n";

        if (overall.avg > 1.0) {
            std::cerr << "WARNING: Average latency exceeded 1ms SLA! (" << overall.avg << " ms)\n";
        } else {
            std::cout << "SUCCESS: Performance SLA met.\n";
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
        if (latencies.empty()) return {name, 0, 0, 0, 0};

        double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
        double avg = sum / latencies.size();

        std::sort(latencies.begin(), latencies.end());
        double max = latencies.back();
        double p99 = latencies[static_cast<size_t>(latencies.size() * 0.99)];

        return {name, avg, max, p99, latencies.size()};
    }

    void print_stats(const LatencyStats& s) {
        std::cout << std::left << std::setw(30) << s.name 
                  << std::right << std::fixed << std::setprecision(4) 
                  << std::setw(12) << s.avg 
                  << std::setw(12) << s.max 
                  << std::setw(12) << s.p99 
                  << std::setw(10) << s.count << "\n";
    }

    void save_to_csv() {
        std::ofstream csv("benchmark_results.csv");
        csv << "Scenario,AvgMS,MaxMS,P99MS,Keys\n";
        for (const auto& r : results) {
            csv << r.name << "," << r.avg << "," << r.max << "," << r.p99 << "," << r.count << "\n";
        }
        csv.close();
        std::cout << "Full results saved to benchmark_results.csv\n";
    }
};

int main() {
    BenchmarkSuite suite("Realistic Typing Simulation");

    // 1. Standard Telex - Formal Vietnamese
    // Sequence: "lotus engine laf mootj bbooj goox tieengs vieetj hieenj ddaaij..."
    std::string telex_formal = 
        "lotus engine laf mootj bbooj goox tieengs vieetj hieenj ddaaij, "
        "toois ưu hoas cho hieeuu nng dda rrat cao. "
        "vieecj goox tieengs vieetj chuwa bao gioof deeex daangf ddeens thees.";

    suite.add_scenario("Telex Standard", telex_formal, [](Engine& e) {
        e.set_method(InputMethod::TELEX);
    });

    // 2. Fast Telex (Flexible placement & Auto-restore)
    std::string telex_fast = 
        "viecje goox tieengs vieetj chuwa bao gioof deeex daangf ddeens thees. "
        "dduowcj thieetj kees ddee hoatj ddoongj muowjt ma` treen nhieeu` neen` taangr. "
        "dosd ddaay laf mootj vis duwj veea viecje goox nhanh.";

    suite.add_scenario("Telex Flexible", telex_fast, [](Engine& e) {
        e.set_method(InputMethod::TELEX);
        e.set_auto_restore(true);
    });

    // 3. VNI Standard
    // Sequence uses VNI number keys for marks
    std::string vni_formal = 
        "lotus engine la2 mo^6t5 bo^65 go~4 tie^6ng1 vie^6t5 hie^6n5 dda9i5, "
        "to^6i1 u*7u hoas1 cho hie^6u65 na8ng dda~4 ra^6t1 cao. "
        "vie^6c5 go^64 tie^6ng1 vie^6t5 chu*7a bao gio*72 de^6~4 da`2ng dde^6n1 the^61 .";

    suite.add_scenario("VNI Standard", vni_formal, [](Engine& e) {
        e.set_method(InputMethod::VNI);
    });

    // 4. Mixed Language (English Technical Terms)
    std::string mixed = 
        "Trong lapj trình C++, vieecj manage memory laf rrat' quan trongj. "
        "Sử dụng smart pointers giúp avoid memory leaks và dangling pointers. "
        "Engine na`y dduowcj optimize rrat' kỹ cho low-latency applications.";

    suite.add_scenario("Mixed English/VN", mixed, [](Engine& e) {
        e.set_method(InputMethod::TELEX);
        e.set_auto_restore(true);
    });

    // 5. Editing & Correction (Simulating backspaces)
    std::string editing = 
        "xin chaof\b\b\b\b\boof "  // typo correction
        "mootj\b\b\b\b\btj "      
        "truowngf\b\b\b\b\b\b\b\bngf "; // full retype

    suite.add_scenario("Editing (Backspaces)", editing, [](Engine& e) {
        e.set_method(InputMethod::TELEX);
    });

    // 6. Macros & Shortcuts
    suite.add_scenario("Macros/Shortcuts", "vn hcm tks bst ", [](Engine& e) {
        e.set_method(InputMethod::TELEX);
        e.add_shortcut("vn", "Việt Nam");
        e.add_shortcut("hcm", "Thành phố Hồ Chí Minh");
        e.add_shortcut("tks", "Cảm ơn");
        e.add_shortcut("bst", "Bộ sưu tập");
    });

    // 7. Long Syllable Stress Test
    std::string stress = "nghieengs dduowngf tuyeens quyeet' nghieenj huyeexnh ";
    for(int i=0; i<5; ++i) stress += stress; // Increase volume

    suite.add_scenario("Stress (Long Syllables)", stress, [](Engine& e) {
        e.set_method(InputMethod::TELEX);
    });

    // 8. Customized Session (Multiple features enabled)
    std::string customized = 
        "wow mootj buoori sangs ddepj troi`. " // w -> ư (free_w)
        "Goox tieengs Vieetj thajt vui.  "     // double space -> period
        "anh dda~ dduwowjc ddawng kys.";        // std_uo (uo -> ươ)
    
    suite.add_scenario("Customized Session", customized, [](Engine& e) {
        e.set_method(InputMethod::TELEX);
        e.set_free_w(FreeWOption::ALWAYS);
        e.set_std_uo(true);
        e.set_double_space_to_period(true);
        e.set_auto_capitalize(true);
    });

    suite.run_all();

    return 0;
}
