#include <chrono>
#include <map>
#include <stack>
#include <string>

class Profiler {
   private:
    struct FunctionStats {
        long long totalTime = 0;  // in microseconds
        int calls = 0;
    };
    static inline std::map<std::string, FunctionStats> stats;
    static inline std::stack<
        std::pair<std::string, std::chrono::high_resolution_clock::time_point>>
        callStack;

   public:
    class ScopedProfiler {
       private:
        std::string functionName;

       public:
        ScopedProfiler(const std::string& name);
        ~ScopedProfiler();
    };

    static void printResults();
    static void reset();
};

// Macro to make profiling easier
#define PROFILE_FUNCTION() Profiler::ScopedProfiler profiler(__FUNCTION__)
