#pragma once

#include <string>
#include <vector>
#include <functional>
#include "../../utils/Console.hpp"

namespace StockMarketSimulator {

class MenuItem {
private:
    std::string text;
    std::function<void()> callback;
    bool enabled;

public:
    MenuItem(const std::string& text, std::function<void()> callback = nullptr, bool enabled = true);

    std::string getText() const;
    void setText(const std::string& text);
    
    std::function<void()> getCallback() const;
    void setCallback(std::function<void()> callback);
    
    bool isEnabled() const;
    void setEnabled(bool enabled);
    
    void execute() const;
};

class Menu {
private:
    std::vector<MenuItem> items;
    std::string title;
    int x, y;
    int width;
    int selectedIndex;
    TextColor normalFg, normalBg;
    TextColor selectedFg, selectedBg;
    TextColor titleFg, titleBg;
    bool visible;
    bool exitOnSelect;

public:
    Menu();
    Menu(const std::string& title, int x = 0, int y = 0);

    void addItem(const MenuItem& item);
    void addItem(const std::string& text, std::function<void()> callback = nullptr, bool enabled = true);
    void removeItem(int index);
    void clearItems();
    
    size_t getItemCount() const;
    MenuItem& getItem(int index);
    const MenuItem& getItem(int index) const;
    
    void setTitle(const std::string& title);
    std::string getTitle() const;
    
    void setPosition(int x, int y);
    int getX() const;
    int getY() const;
    
    void setWidth(int width);
    int getWidth() const;
    
    void setSelectedIndex(int index);
    int getSelectedIndex() const;
    
    void setNormalColors(TextColor fg, TextColor bg);
    void setSelectedColors(TextColor fg, TextColor bg);
    void setTitleColors(TextColor fg, TextColor bg);
    
    TextColor getNormalFg() const;
    TextColor getNormalBg() const;
    TextColor getSelectedFg() const;
    TextColor getSelectedBg() const;
    TextColor getTitleFg() const;
    TextColor getTitleBg() const;
    
    void setVisible(bool visible);
    bool isVisible() const;
    
    void setExitOnSelect(bool exitOnSelect);
    bool getExitOnSelect() const;
    
    void selectNext();
    void selectPrevious();
    bool selectItem(int index);
    
    void executeSelected();
    
    void draw() const;
    int handleInput() const;
    void run();
};

}