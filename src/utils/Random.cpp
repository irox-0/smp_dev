#include "Random.hpp"

namespace StockMarketSimulator {

    std::mt19937 Random::generator;
    bool Random::isInitialized = false;

    void Random::initialize(unsigned int seed) {
        generator.seed(seed);
        isInitialized = true;
    }

    int Random::getInt(int min, int max) {
        if (!isInitialized) {
            initialize();
        }

        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(generator);
    }

    double Random::getDouble(double min, double max) {
        if (!isInitialized) {
            initialize();
        }

        std::uniform_real_distribution<double> distribution(min, max);
        return distribution(generator);
    }

    bool Random::getBool(double probability) {
        if (!isInitialized) {
            initialize();
        }

        if (probability <= 0.0) return false;
        if (probability >= 1.0) return true;

        std::bernoulli_distribution distribution(probability);
        return distribution(generator);
    }

    size_t Random::getIndex(size_t size) {
        if (size == 0) {
            throw std::runtime_error("Cannot generate index for size 0");
        }

        if (!isInitialized) {
            initialize();
        }

        std::uniform_int_distribution<size_t> distribution(0, size - 1);
        return distribution(generator);
    }

    double Random::getNormal(double mean, double stdDev) {
        if (!isInitialized) {
            initialize();
        }

        std::normal_distribution<double> distribution(mean, stdDev);
        return distribution(generator);
    }

}