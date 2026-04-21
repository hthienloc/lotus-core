#include "lotus_engine/engine.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <numeric>
#include <algorithm>

using namespace lotus_engine;

int main() {
    Engine engine;
    engine.set_method(InputMethod::TELEX);
    engine.set_tone_style(ToneStyle::NEW);

    const int NUM_KEYSTROKES = 10000;
    std::vector<double> latencies_ms;
    latencies_ms.reserve(NUM_KEYSTROKES);

    // Generate a simulated typing sequence
    std::string sequence = "truongwf ddaji hocj bachs khoa ddaf nangwx ";
    std::string full_text = "";
    while (full_text.length() < NUM_KEYSTROKES) {
        full_text += sequence;
    }
    full_text.resize(NUM_KEYSTROKES);

    std::cout << "Starting benchmark with " << NUM_KEYSTROKES << " keypresses..." << std::endl;

    for (char c : full_text) {
        auto start = std::chrono::high_resolution_clock::now();
        
        engine.process_key(c, {});
        
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> diff = end - start;
        latencies_ms.push_back(diff.count());
    }

    double sum = std::accumulate(latencies_ms.begin(), latencies_ms.end(), 0.0);
    double avg = sum / latencies_ms.size();
    
    auto max_it = std::max_element(latencies_ms.begin(), latencies_ms.end());
    double max_latency = *max_it;

    std::cout << "Target: < 1.0 ms / keypress" << std::endl;
    std::cout << "Average Latency: " << avg << " ms" << std::endl;
    std::cout << "Max Latency: " << max_latency << " ms" << std::endl;

    if (max_latency > 1.0) {
        std::cerr << "WARNING: Max latency exceeded 1ms! (" << max_latency << " ms)" << std::endl;
    } else {
        std::cout << "SUCCESS: Latency SLA met." << std::endl;
    }

    // Output CSV
    std::ofstream csv("benchmark_results.csv");
    csv << "Keypress,LatencyMS\n";
    for (size_t i = 0; i < latencies_ms.size(); i++) {
        csv << i << "," << latencies_ms[i] << "\n";
    }
    csv.close();
    std::cout << "Results written to benchmark_results.csv." << std::endl;

    return 0;
}
