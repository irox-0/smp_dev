#include <gtest/gtest.h>
#include "../../src/utils/Random.hpp"

using namespace StockMarketSimulator;

TEST(RandomTest, InitializeTest) {
    Random::initialize(42);

    ASSERT_NO_THROW(Random::initialize(42));
}

TEST(RandomTest, GetIntTest) {
    Random::initialize(42);

    for (int i = 0; i < 1000; ++i) {
        int value = Random::getInt(10, 20);
        ASSERT_GE(value, 10);
        ASSERT_LE(value, 20);
    }

    Random::initialize(42);
    int first = Random::getInt(0, 1000);
    Random::initialize(42);
    int second = Random::getInt(0, 1000);
    ASSERT_EQ(first, second);
}

TEST(RandomTest, GetDoubleTest) {
    Random::initialize(42);

    for (int i = 0; i < 1000; ++i) {
        double value = Random::getDouble(5.0, 10.0);
        ASSERT_GE(value, 5.0);
        ASSERT_LE(value, 10.0);
    }

    Random::initialize(42);
    double first = Random::getDouble(0.0, 1.0);
    Random::initialize(42);
    double second = Random::getDouble(0.0, 1.0);
    ASSERT_DOUBLE_EQ(first, second);
}

TEST(RandomTest, GetBoolTest) {
    Random::initialize(42);

    ASSERT_FALSE(Random::getBool(0.0));
    ASSERT_TRUE(Random::getBool(1.0));

    int trueCount = 0;
    int iterationCount = 10000;

    for (int i = 0; i < iterationCount; ++i) {
        if (Random::getBool(0.5)) {
            trueCount++;
        }
    }

    double ratio = static_cast<double>(trueCount) / iterationCount;
    ASSERT_NEAR(ratio, 0.5, 0.05);
}

TEST(RandomTest, GetIndexTest) {
    Random::initialize(42);

    size_t size = 10;
    for (int i = 0; i < 1000; ++i) {
        size_t index = Random::getIndex(size);
        ASSERT_LT(index, size);
    }

    ASSERT_THROW(Random::getIndex(0), std::runtime_error);
}

TEST(RandomTest, GetNormalTest) {
    Random::initialize(42);

    double mean = 5.0;
    double stdDev = 2.0;
    double sum = 0.0;
    int count = 10000;

    for (int i = 0; i < count; ++i) {
        sum += Random::getNormal(mean, stdDev);
    }

    double average = sum / count;
    ASSERT_NEAR(average, mean, mean * 0.1);
}

TEST(RandomTest, GetRandomElementTest) {
    Random::initialize(42);

    std::vector<int> items = {1, 2, 3, 4, 5};

    for (int i = 0; i < 100; ++i) {
        int element = Random::getRandomElement(items);
        ASSERT_TRUE(std::find(items.begin(), items.end(), element) != items.end());
    }

    std::vector<int> emptyItems;
    ASSERT_THROW(Random::getRandomElement(emptyItems), std::runtime_error);
}

TEST(RandomTest, GetWeightedRandomElementTest) {
    Random::initialize(42);

    std::vector<int> items = {1, 2, 3, 4, 5};
    std::vector<double> weights = {0.1, 0.2, 0.3, 0.3, 0.1};

    for (int i = 0; i < 100; ++i) {
        int element = Random::getWeightedRandomElement(items, weights);
        ASSERT_TRUE(std::find(items.begin(), items.end(), element) != items.end());
    }

    std::vector<int> emptyItems;
    std::vector<double> emptyWeights;

    ASSERT_THROW(Random::getWeightedRandomElement(emptyItems, weights), std::runtime_error);
    ASSERT_THROW(Random::getWeightedRandomElement(items, emptyWeights), std::runtime_error);

    std::vector<double> incorrectWeights = {0.1, 0.2};
    ASSERT_THROW(Random::getWeightedRandomElement(items, incorrectWeights), std::runtime_error);
}

TEST(RandomTest, ShuffleTest) {
    Random::initialize(42);

    std::vector<int> original = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> shuffled = original;

    Random::shuffle(shuffled);

    ASSERT_EQ(original.size(), shuffled.size());

    for (int item : original) {
        ASSERT_TRUE(std::find(shuffled.begin(), shuffled.end(), item) != shuffled.end());
    }

    bool orderChanged = false;
    for (size_t i = 0; i < original.size(); ++i) {
        if (original[i] != shuffled[i]) {
            orderChanged = true;
            break;
        }
    }

    ASSERT_TRUE(orderChanged);
}