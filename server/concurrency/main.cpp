#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

int globalCounter = 0;
std::mutex counterMutex;

void incrementManyTimes(int times) {
    for (int i = 0; i < times; ++i) {
        std::unique_lock<std::mutex> guard(counterMutex);
        globalCounter++;
    }
}

int main() {
    const int numThreads = 4;
    const int incrementsPerThread = 100000;

    std::vector<std::thread> threads;

    /* Starts several threads */
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(incrementManyTimes, incrementsPerThread);
    }

    /* Join all threads */
    for (auto &t : threads) {
        t.join();
    }

    std::cout << "Expected: " << numThreads * incrementsPerThread << "\n";
    std::cout << "Actual:   " << globalCounter << "\n";

    return 0;
}
