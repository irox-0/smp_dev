#include <gtest/gtest.h>
#include "ui/Screen.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

class TestScreen : public Screen {
protected:
    void drawContent() const override {
        Console::setCursorPosition(getX() + 2, getY() + 2);
        Console::setColor(TextColor::White);
        Console::print("Test content");
    }

public:
    TestScreen() : Screen("Test Screen") {}

    bool handleInput(int key) override {
        if (key == static_cast<int>(Key::Escape)) {
            close();
            return false;
        }
        return true;
    }
};

class ScreenTest : public ::testing::Test {
protected:
    void SetUp() override {
        screen = std::make_unique<TestScreen>();
    }

    std::unique_ptr<TestScreen> screen;
};

TEST_F(ScreenTest, InitializationTest) {
    ASSERT_EQ(screen->getTitle(), "Test Screen");
    ASSERT_EQ(screen->getX(), 0);
    ASSERT_EQ(screen->getY(), 0);
    ASSERT_EQ(screen->getWidth(), 80);
    ASSERT_EQ(screen->getHeight(), 24);
    ASSERT_TRUE(screen->isVisible());
    ASSERT_FALSE(screen->isActive());
    ASSERT_EQ(screen->getTitleFg(), TextColor::White);
    ASSERT_EQ(screen->getTitleBg(), TextColor::Blue);
    ASSERT_EQ(screen->getBodyFg(), TextColor::Default);
    ASSERT_EQ(screen->getBodyBg(), TextColor::Default);
    ASSERT_EQ(screen->getType(), ScreenType::Custom);
}

TEST_F(ScreenTest, SettersTest) {
    screen->setTitle("New Title");
    ASSERT_EQ(screen->getTitle(), "New Title");

    screen->setPosition(10, 10);
    ASSERT_EQ(screen->getX(), 10);
    ASSERT_EQ(screen->getY(), 10);

    screen->setSize(60, 20);
    ASSERT_EQ(screen->getWidth(), 60);
    ASSERT_EQ(screen->getHeight(), 20);

    screen->setVisible(false);
    ASSERT_FALSE(screen->isVisible());

    screen->setActive(true);
    ASSERT_TRUE(screen->isActive());

    screen->setTitleColors(TextColor::Red, TextColor::Green);
    ASSERT_EQ(screen->getTitleFg(), TextColor::Red);
    ASSERT_EQ(screen->getTitleBg(), TextColor::Green);

    screen->setBodyColors(TextColor::Yellow, TextColor::Magenta);
    ASSERT_EQ(screen->getBodyFg(), TextColor::Yellow);
    ASSERT_EQ(screen->getBodyBg(), TextColor::Magenta);
}

TEST_F(ScreenTest, InvalidSizeTest) {
    screen->setSize(-10, -5);
    ASSERT_EQ(screen->getWidth(), 80);
    ASSERT_EQ(screen->getHeight(), 24);
}

TEST_F(ScreenTest, DrawTest) {
    ASSERT_NO_THROW(screen->draw());

    screen->setVisible(false);
    ASSERT_NO_THROW(screen->draw());
}

TEST_F(ScreenTest, HandlInputTest) {
    bool result = screen->handleInput(static_cast<int>(Key::Enter));
    ASSERT_TRUE(result);

    result = screen->handleInput(static_cast<int>(Key::Escape));
    ASSERT_FALSE(result);
    ASSERT_FALSE(screen->isActive());
}

TEST_F(ScreenTest, CloseTest) {
    screen->setActive(true);
    ASSERT_TRUE(screen->isActive());

    screen->close();
    ASSERT_FALSE(screen->isActive());
}

class MockScreen : public Screen {
protected:
    mutable int drawCount = 0;
    int updateCount = 0;

    void drawContent() const override {
        drawCount++;
    }

public:
    MockScreen() : Screen("Mock Screen") {}

    void update() override {
        updateCount++;
        if (updateCount >= 3) {
            close();
        }
    }

    bool handleInput(int key) override {
        return true;
    }

    int getDrawCount() const {
        return drawCount;
    }

    int getUpdateCount() const {
        return updateCount;
    }

    void run() override {
        active = true;
        draw();
        update();
        active = false;
    }
};

TEST_F(ScreenTest, RunTest) {
    MockScreen mockScreen;
    mockScreen.run();

    ASSERT_GE(mockScreen.getDrawCount(), 1);
    ASSERT_GE(mockScreen.getUpdateCount(), 1);
    ASSERT_FALSE(mockScreen.isActive());
}