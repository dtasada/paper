#include "../include/Profiler.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <stack>
#include <string>

Profiler::ScopedProfiler::ScopedProfiler(const std::string& name) : functionName(name) {
    callStack.push({functionName, std::chrono::high_resolution_clock::now()});
}

Profiler::ScopedProfiler::~ScopedProfiler() {
    auto end = std::chrono::high_resolution_clock::now();
    auto [name, start] = callStack.top();
    callStack.pop();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    stats[name].totalTime += duration;
    stats[name].calls++;
}

void Profiler::printResults() {
    std::cout << "\nProfiling Results:\n";
    std::cout << std::setw(30) << "Function Name" << std::setw(15) << "Calls" << std::setw(15)
              << "Total Time" << std::setw(15) << "Avg Time\n";
    std::cout << std::string(75, '-') << "\n";

    for (const auto& [name, data] : stats) {
        double avgTime = data.calls > 0 ? static_cast<double>(data.totalTime) / data.calls : 0;
        std::cout << std::setw(30) << name << std::setw(15) << data.calls << std::setw(15)
                  << data.totalTime << "μs" << std::setw(15) << std::fixed << std::setprecision(2)
                  << avgTime << "μs\n";
    }
}

void Profiler::reset() {
    stats.clear();
    while (!callStack.empty()) callStack.pop();
}
