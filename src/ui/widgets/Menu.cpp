#include "Menu.hpp"
#include <algorithm>

namespace StockMarketSimulator {

// MenuItem implementation
MenuItem::MenuItem(const std::string& text, std::function<void()> callback, bool enabled)
    : text(text), callback(callback), enabled(enabled)
{
}

std::string MenuItem::getText() const {
    return text;
}

void MenuItem::setText(const std::string& text) {
    this->text = text;
}

std::function<void()> MenuItem::getCallback() const {
    return callback;
}

void MenuItem::setCallback(std::function<void()> callback) {
    this->callback = callback;
}

bool MenuItem::isEnabled() const {
    return enabled;
}

void MenuItem::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void MenuItem::execute() const {
    if (enabled && callback) {
        callback();
    }
}

// Menu implementation
Menu::Menu()
    : title(""), x(0), y(0), width(40), selectedIndex(0),
      normalFg(TextColor::White), normalBg(TextColor::Default),
      selectedFg(TextColor::Black), selectedBg(TextColor::White),
      titleFg(TextColor::White), titleBg(TextColor::Blue),
      visible(true), exitOnSelect(true)
{
}

Menu::Menu(const std::string& title, int x, int y)
    : title(title), x(x), y(y), width(40), selectedIndex(0),
      normalFg(TextColor::White), normalBg(TextColor::Default),
      selectedFg(TextColor::Black), selectedBg(TextColor::White),
      titleFg(TextColor::White), titleBg(TextColor::Blue),
      visible(true), exitOnSelect(true)
{
}

void Menu::addItem(const MenuItem& item) {
    items.push_back(item);
}

void Menu::addItem(const std::string& text, std::function<void()> callback, bool enabled) {
    items.emplace_back(text, callback, enabled);
}

void Menu::removeItem(int index) {
    if (index >= 0 && index < static_cast<int>(items.size())) {
        items.erase(items.begin() + index);
        if (selectedIndex >= static_cast<int>(items.size())) {
            selectedIndex = static_cast<int>(items.size()) - 1;
        }
        if (selectedIndex < 0) {
            selectedIndex = 0;
        }
    }
}

void Menu::clearItems() {
    items.clear();
    selectedIndex = 0;
}

size_t Menu::getItemCount() const {
    return items.size();
}

MenuItem& Menu::getItem(int index) {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        throw std::out_of_range("Menu item index out of range");
    }
    return items[index];
}

const MenuItem& Menu::getItem(int index) const {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        throw std::out_of_range("Menu item index out of range");
    }
    return items[index];
}

void Menu::setTitle(const std::string& title) {
    this->title = title;
}

std::string Menu::getTitle() const {
    return title;
}

void Menu::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

int Menu::getX() const {
    return x;
}

int Menu::getY() const {
    return y;
}

void Menu::setWidth(int width) {
    if (width > 0) {
        this->width = width;
    }
}

int Menu::getWidth() const {
    return width;
}

void Menu::setSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(items.size())) {
        selectedIndex = index;
    }
}

int Menu::getSelectedIndex() const {
    return selectedIndex;
}

void Menu::setNormalColors(TextColor fg, TextColor bg) {
    normalFg = fg;
    normalBg = bg;
}

void Menu::setSelectedColors(TextColor fg, TextColor bg) {
    selectedFg = fg;
    selectedBg = bg;
}

void Menu::setTitleColors(TextColor fg, TextColor bg) {
    titleFg = fg;
    titleBg = bg;
}

TextColor Menu::getNormalFg() const {
    return normalFg;
}

TextColor Menu::getNormalBg() const {
    return normalBg;
}

TextColor Menu::getSelectedFg() const {
    return selectedFg;
}

TextColor Menu::getSelectedBg() const {
    return selectedBg;
}

TextColor Menu::getTitleFg() const {
    return titleFg;
}

TextColor Menu::getTitleBg() const {
    return titleBg;
}

void Menu::setVisible(bool visible) {
    this->visible = visible;
}

bool Menu::isVisible() const {
    return visible;
}

void Menu::setExitOnSelect(bool exitOnSelect) {
    this->exitOnSelect = exitOnSelect;
}

bool Menu::getExitOnSelect() const {
    return exitOnSelect;
}

void Menu::selectNext() {
    if (items.empty()) return;
    
    int startIndex = selectedIndex;
    do {
        selectedIndex = (selectedIndex + 1) % items.size();
        if (items[selectedIndex].isEnabled() || selectedIndex == startIndex) {
            break;
        }
    } while (true);
}

void Menu::selectPrevious() {
    if (items.empty()) return;
    
    int startIndex = selectedIndex;
    do {
        selectedIndex = (selectedIndex - 1 + items.size()) % items.size();
        if (items[selectedIndex].isEnabled() || selectedIndex == startIndex) {
            break;
        }
    } while (true);
}

bool Menu::selectItem(int index) {
    if (index >= 0 && index < static_cast<int>(items.size()) && items[index].isEnabled()) {
        selectedIndex = index;
        return true;
    }
    return false;
}

void Menu::executeSelected() {
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size())) {
        items[selectedIndex].execute();
    }
}

    void Menu::draw() const {
    if (!visible) return;

    // Draw title
    if (!title.empty()) {
        Console::setCursorPosition(x, y);
        Console::setColor(titleFg, titleBg);

        // Handle title formatting more safely
        std::string displayTitle;

        if (title.length() >= static_cast<size_t>(width)) {
            // Title is too long, truncate it
            displayTitle = title.substr(0, width);
        } else {
            // Calculate padding to center the title (safely)
            int titleLen = static_cast<int>(title.length());
            int leftPadding = std::max(0, (width - titleLen) / 2);
            int rightPadding = std::max(0, width - titleLen - leftPadding);

            displayTitle = std::string(leftPadding, ' ') + title + std::string(rightPadding, ' ');
        }

        Console::print(displayTitle);
        Console::resetAttributes();
    }

    // Draw menu items (similarly update the item formatting)
    for (size_t i = 0; i < items.size(); i++) {
        Console::setCursorPosition(x, y + (title.empty() ? 0 : 1) + static_cast<int>(i));

        if (static_cast<int>(i) == selectedIndex) {
            Console::setColor(selectedFg, selectedBg);
        } else {
            Console::setColor(normalFg, normalBg);
        }

        std::string itemText = (items[i].isEnabled() ? "" : "[Disabled] ") + items[i].getText();
        std::string displayText;

        if (itemText.length() >= static_cast<size_t>(width)) {
            // Text is too long, truncate it
            displayText = itemText.substr(0, width - 3) + "...";
        } else {
            // Pad to width with spaces
            displayText = itemText + std::string(width - itemText.length(), ' ');
        }

        Console::print(displayText);
    }

    Console::resetAttributes();
}
int Menu::handleInput() const {
    char key = Console::readChar();
    return static_cast<int>(key);
}

void Menu::run() {
    if (!visible || items.empty()) return;
    
    bool running = true;
    draw();
    
    while (running) {
        int key = handleInput();
        
        switch (key) {
            case static_cast<int>(Key::ArrowUp):
                const_cast<Menu*>(this)->selectPrevious();
                break;
                
            case static_cast<int>(Key::ArrowDown):
                const_cast<Menu*>(this)->selectNext();
                break;
                
            case static_cast<int>(Key::Enter):
                executeSelected();
                if (exitOnSelect) {
                    running = false;
                }
                break;
                
            case static_cast<int>(Key::Escape):
                running = false;
                break;
                
            default:
                // Handle numeric keys
                if (key >= '0' && key <= '9') {
                    int index = key - '0';
                    if (index == 0) index = 10;  // Handle 0 as the 10th item
                    else index--;  // Adjust for 1-based indexing in the menu display
                    
                    if (index >= 0 && index < static_cast<int>(items.size()) && 
                        items[index].isEnabled()) {
                        const_cast<Menu*>(this)->setSelectedIndex(index);
                        executeSelected();
                        if (exitOnSelect) {
                            running = false;
                        }
                    }
                }
                break;
        }
        
        draw();
    }
}

}