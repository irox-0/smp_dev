#pragma once

#include <random>
#include <ctime>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace StockMarketSimulator {

class Random {
private:
    static std::mt19937 generator;

    static bool isInitialized;

public:
    static void initialize(unsigned int seed = static_cast<unsigned int>(std::time(nullptr)));

    static int getInt(int min, int max);

    static double getDouble(double min, double max);

    static bool getBool(double probability = 0.5);

    template<typename T>
    static T getRandomElement(const std::vector<T>& items) {
        if (items.empty()) {
            throw std::runtime_error("Cannot select from an empty vector");
        }

        if (!isInitialized) {
            initialize();
        }

        std::uniform_int_distribution<size_t> distribution(0, items.size() - 1);
        return items[distribution(generator)];
    }

    template<typename T>
    static T getWeightedRandomElement(const std::vector<T>& items, const std::vector<double>& weights) {
        if (items.empty() || weights.empty() || items.size() != weights.size()) {
            throw std::runtime_error("Invalid input for weighted random selection");
        }

        if (!isInitialized) {
            initialize();
        }

        std::discrete_distribution<size_t> distribution(weights.begin(), weights.end());
        return items[distribution(generator)];
    }

    static size_t getIndex(size_t size);

    static double getNormal(double mean, double stdDev);

    template<typename T>
    static void shuffle(std::vector<T>& items) {
        if (!isInitialized) {
            initialize();
        }

        std::shuffle(items.begin(), items.end(), generator);
    }
};

}
