#include "Screen.hpp"
#include <algorithm>

namespace StockMarketSimulator {

Screen::Screen()
    : title(""), x(0), y(0), width(80), height(24),
      visible(true), active(false),
      titleFg(TextColor::White), titleBg(TextColor::Blue),
      bodyFg(TextColor::Default), bodyBg(TextColor::Default),
      type(ScreenType::Custom)
{
}

Screen::Screen(const std::string& title, ScreenType type)
    : title(title), x(0), y(0), width(80), height(24),
      visible(true), active(false),
      titleFg(TextColor::White), titleBg(TextColor::Blue),
      bodyFg(TextColor::Default), bodyBg(TextColor::Default),
      type(type)
{
}

void Screen::setTitle(const std::string& title) {
    this->title = title;
}

std::string Screen::getTitle() const {
    return title;
}

void Screen::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

int Screen::getX() const {
    return x;
}

int Screen::getY() const {
    return y;
}

void Screen::setSize(int width, int height) {
    if (width > 0 && height > 0) {
        this->width = width;
        this->height = height;
    }
}

int Screen::getWidth() const {
    return width;
}

int Screen::getHeight() const {
    return height;
}

void Screen::setVisible(bool visible) {
    this->visible = visible;
}

bool Screen::isVisible() const {
    return visible;
}

void Screen::setActive(bool active) {
    this->active = active;
}

bool Screen::isActive() const {
    return active;
}

void Screen::setTitleColors(TextColor fg, TextColor bg) {
    titleFg = fg;
    titleBg = bg;
}

void Screen::setBodyColors(TextColor fg, TextColor bg) {
    bodyFg = fg;
    bodyBg = bg;
}

TextColor Screen::getTitleFg() const {
    return titleFg;
}

TextColor Screen::getTitleBg() const {
    return titleBg;
}

TextColor Screen::getBodyFg() const {
    return bodyFg;
}

TextColor Screen::getBodyBg() const {
    return bodyBg;
}

ScreenType Screen::getType() const {
    return type;
}

void Screen::setGame(std::weak_ptr<Game> game) {
    this->game = game;
}

void Screen::setMarket(std::weak_ptr<Market> market) {
    this->market = market;
}

void Screen::setPlayer(std::weak_ptr<Player> player) {
    this->player = player;
}

void Screen::initialize() {
}

void Screen::update() {
}

void Screen::drawTitle() const {
    if (!title.empty()) {
        Console::setCursorPosition(x, y);
        Console::setColor(titleFg, titleBg);

        int titleLen = static_cast<int>(title.length());
        int leftPadding = std::max(0, (width - titleLen) / 2);

        std::string formattedTitle = std::string(leftPadding, ' ') + title;
        formattedTitle += std::string(width - formattedTitle.length(), ' ');

        Console::print(formattedTitle);
        Console::resetAttributes();
    }
}

void Screen::drawBorder() const {
    Console::setColor(bodyFg, bodyBg);
    Console::drawBox(x, y + (title.empty() ? 0 : 1), width, height - (title.empty() ? 0 : 1));
    Console::resetAttributes();
}

void Screen::draw() const {
    if (!visible) return;

    Console::setColor(bodyFg, bodyBg);
    for (int i = 0; i < height; ++i) {
        Console::setCursorPosition(x, y + i);
        Console::print(std::string(width, ' '));
    }

    drawTitle();

    drawBorder();

    drawContent();

    Console::resetAttributes();
}

bool Screen::handleInput(int key) {
    return false;
}

void Screen::run() {
    if (!visible) return;

    active = true;
    draw();

    while (active) {
        if (Console::keyPressed()) {
            int key = Console::readChar();
            if (!handleInput(key)) {
                break;
            }
            draw();
        }
        update();

        Console::sleep(50);
    }

    active = false;
}

void Screen::close() {
    active = false;
}

}