#include <gtest/gtest.h>
#include "ui/widgets/Menu.hpp"
#include "utils/Console.hpp"

using namespace StockMarketSimulator;

class MenuTest : public ::testing::Test {
protected:
    void SetUp() override {
        menu = std::make_unique<Menu>("Test Menu", 0, 0);
        callbackExecuted = false;
    }

    std::unique_ptr<Menu> menu;
    bool callbackExecuted;

    std::function<void()> createCallback() {
        return [this]() { callbackExecuted = true; };
    }
};

TEST_F(MenuTest, DefaultConstructorTest) {
    Menu defaultMenu;

    ASSERT_EQ(defaultMenu.getTitle(), "");
    ASSERT_EQ(defaultMenu.getX(), 0);
    ASSERT_EQ(defaultMenu.getY(), 0);
    ASSERT_EQ(defaultMenu.getWidth(), 40);
    ASSERT_EQ(defaultMenu.getSelectedIndex(), 0);
    ASSERT_EQ(defaultMenu.getItemCount(), 0);
    ASSERT_TRUE(defaultMenu.isVisible());
    ASSERT_TRUE(defaultMenu.getExitOnSelect());
}

TEST_F(MenuTest, ParameterizedConstructorTest) {
    Menu paramMenu("Test Title", 5, 10);

    ASSERT_EQ(paramMenu.getTitle(), "Test Title");
    ASSERT_EQ(paramMenu.getX(), 5);
    ASSERT_EQ(paramMenu.getY(), 10);
    ASSERT_EQ(paramMenu.getWidth(), 40);
    ASSERT_EQ(paramMenu.getSelectedIndex(), 0);
    ASSERT_EQ(paramMenu.getItemCount(), 0);
    ASSERT_TRUE(paramMenu.isVisible());
    ASSERT_TRUE(paramMenu.getExitOnSelect());
}

TEST_F(MenuTest, AddItemTest) {
    menu->addItem("Item 1", createCallback());
    menu->addItem("Item 2");

    ASSERT_EQ(menu->getItemCount(), 2);
    ASSERT_EQ(menu->getItem(0).getText(), "Item 1");
    ASSERT_EQ(menu->getItem(1).getText(), "Item 2");

    MenuItem newItem("Item 3", createCallback());
    menu->addItem(newItem);

    ASSERT_EQ(menu->getItemCount(), 3);
    ASSERT_EQ(menu->getItem(2).getText(), "Item 3");
}

TEST_F(MenuTest, RemoveItemTest) {
    menu->addItem("Item 1");
    menu->addItem("Item 2");
    menu->addItem("Item 3");

    ASSERT_EQ(menu->getItemCount(), 3);

    menu->removeItem(1);

    ASSERT_EQ(menu->getItemCount(), 2);
    ASSERT_EQ(menu->getItem(0).getText(), "Item 1");
    ASSERT_EQ(menu->getItem(1).getText(), "Item 3");

    menu->removeItem(10);
    ASSERT_EQ(menu->getItemCount(), 2);
}

TEST_F(MenuTest, ClearItemsTest) {
    menu->addItem("Item 1");
    menu->addItem("Item 2");

    ASSERT_EQ(menu->getItemCount(), 2);

    menu->clearItems();

    ASSERT_EQ(menu->getItemCount(), 0);
}

TEST_F(MenuTest, GetItemExceptionTest) {
    ASSERT_THROW(menu->getItem(0), std::out_of_range);

    menu->addItem("Item 1");

    ASSERT_NO_THROW(menu->getItem(0));
    ASSERT_THROW(menu->getItem(1), std::out_of_range);
    ASSERT_THROW(menu->getItem(-1), std::out_of_range);
}

TEST_F(MenuTest, SettersTest) {
    menu->setTitle("New Title");
    menu->setPosition(10, 15);
    menu->setWidth(50);
    menu->setExitOnSelect(false);
    menu->setVisible(false);

    ASSERT_EQ(menu->getTitle(), "New Title");
    ASSERT_EQ(menu->getX(), 10);
    ASSERT_EQ(menu->getY(), 15);
    ASSERT_EQ(menu->getWidth(), 50);
    ASSERT_FALSE(menu->getExitOnSelect());
    ASSERT_FALSE(menu->isVisible());

    menu->setWidth(-10);
    ASSERT_EQ(menu->getWidth(), 50);
}

TEST_F(MenuTest, ColorSettersTest) {
    menu->setNormalColors(TextColor::Red, TextColor::Blue);
    menu->setSelectedColors(TextColor::Green, TextColor::Yellow);
    menu->setTitleColors(TextColor::Cyan, TextColor::Magenta);

    ASSERT_EQ(menu->getNormalFg(), TextColor::Red);
    ASSERT_EQ(menu->getNormalBg(), TextColor::Blue);
    ASSERT_EQ(menu->getSelectedFg(), TextColor::Green);
    ASSERT_EQ(menu->getSelectedBg(), TextColor::Yellow);
    ASSERT_EQ(menu->getTitleFg(), TextColor::Cyan);
    ASSERT_EQ(menu->getTitleBg(), TextColor::Magenta);
}

TEST_F(MenuTest, SelectionMethodsTest) {
    menu->addItem("Item 1");
    menu->addItem("Item 2", nullptr, false);
    menu->addItem("Item 3");

    ASSERT_EQ(menu->getSelectedIndex(), 0);

    menu->selectNext();
    ASSERT_EQ(menu->getSelectedIndex(), 2);

    menu->selectNext();
    ASSERT_EQ(menu->getSelectedIndex(), 0);

    menu->selectPrevious();
    ASSERT_EQ(menu->getSelectedIndex(), 2);

    ASSERT_TRUE(menu->selectItem(0));
    ASSERT_EQ(menu->getSelectedIndex(), 0);

    ASSERT_FALSE(menu->selectItem(1));
    ASSERT_EQ(menu->getSelectedIndex(), 0);

    ASSERT_FALSE(menu->selectItem(5));
    ASSERT_EQ(menu->getSelectedIndex(), 0);
}

TEST_F(MenuTest, ExecuteSelectedTest) {
    menu->addItem("Item 1", createCallback());
    menu->addItem("Item 2", nullptr, false);
    menu->addItem("Item 3");

    menu->executeSelected();
    ASSERT_TRUE(callbackExecuted);

    callbackExecuted = false;
    menu->selectItem(2);
    menu->executeSelected();
    ASSERT_FALSE(callbackExecuted);
}

TEST_F(MenuTest, MenuItemTest) {
    MenuItem item("Test Item", createCallback(), true);

    ASSERT_EQ(item.getText(), "Test Item");
    ASSERT_TRUE(item.isEnabled());

    item.execute();
    ASSERT_TRUE(callbackExecuted);

    callbackExecuted = false;
    item.setEnabled(false);

    ASSERT_FALSE(item.isEnabled());

    item.execute();
    ASSERT_FALSE(callbackExecuted);

    item.setEnabled(true);
    item.setText("New Text");

    ASSERT_EQ(item.getText(), "New Text");
    ASSERT_TRUE(item.isEnabled());

    item.setCallback(nullptr);
    item.execute();
    ASSERT_FALSE(callbackExecuted);
}

TEST_F(MenuTest, DrawTest) {
    menu->addItem("Item 1");
    menu->addItem("Item 2");

    ASSERT_NO_THROW(menu->draw());

    menu->setVisible(false);
    ASSERT_NO_THROW(menu->draw());
}

class MenuMock : public Menu {
public:
    MenuMock() : Menu() {}

    int mockHandleInput() const {
        return static_cast<int>(Key::Escape);
    }

    void runMockTest() {
        if (!isVisible() || getItemCount() == 0) return;

        bool running = true;
        draw();

        running = false;
    }
};

TEST_F(MenuTest, RunMockTest) {
    MenuMock mockMenu;
    mockMenu.addItem("Item 1", createCallback());

    ASSERT_NO_THROW(mockMenu.runMockTest());
}