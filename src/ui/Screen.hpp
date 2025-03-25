#pragma once

#include <string>
#include <memory>
#include <functional>
#include "../utils/Console.hpp"

namespace StockMarketSimulator {

class Game;
class Market;
class Player;

enum class ScreenType {
    Main,
    Market,
    Company,
    Portfolio,
    News,
    Financial,
    Custom
};

class Screen {
protected:
    std::string title;
    int x, y;
    int width, height;
    bool visible;
    bool active;
    TextColor titleFg, titleBg;
    TextColor bodyFg, bodyBg;
    ScreenType type;

    std::weak_ptr<Game> game;
    std::weak_ptr<Market> market;
    std::weak_ptr<Player> player;

    virtual void drawTitle() const;
    virtual void drawBorder() const;
    virtual void drawContent() const = 0;

public:
    Screen();
    Screen(const std::string& title, ScreenType type = ScreenType::Custom);
    virtual ~Screen() = default;

    void setTitle(const std::string& title);
    std::string getTitle() const;

    void setPosition(int x, int y);
    int getX() const;
    int getY() const;

    void setSize(int width, int height);
    int getWidth() const;
    int getHeight() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void setActive(bool active);
    bool isActive() const;

    void setTitleColors(TextColor fg, TextColor bg);
    void setBodyColors(TextColor fg, TextColor bg);

    TextColor getTitleFg() const;
    TextColor getTitleBg() const;
    TextColor getBodyFg() const;
    TextColor getBodyBg() const;

    ScreenType getType() const;

    void setGame(std::weak_ptr<Game> game);
    void setMarket(std::weak_ptr<Market> market);
    void setPlayer(std::weak_ptr<Player> player);

    virtual void initialize();
    virtual void update();
    virtual void draw() const;
    virtual bool handleInput(int key);
    virtual void run();
    virtual void close();
};

}