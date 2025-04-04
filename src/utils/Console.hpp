#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#endif

namespace StockMarketSimulator {

enum class TextColor {
    Black = 0,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    Default
};

enum class TextStyle {
    Regular = 0,
    Bold,
    Underline
};

enum class Key {
    Enter = 13,
    Escape = 27,
    Space = 32,
    ArrowUp = 72,
    ArrowDown = 80,
    ArrowLeft = 75,
    ArrowRight = 77
};

class Console {
private:
    static TextColor currentFgColor;
    static TextColor currentBgColor;
    static TextStyle currentStyle;

    static std::string getAnsiColorCode(TextColor fg, TextColor bg, TextStyle style);

    static void initializeConsole();

    static bool isInitialized;

#ifndef _WIN32
    // Store original terminal settings to restore them later
    static struct termios originalTermios;
    static bool termiosSet;
    static void setupTerminalMode();
    static void restoreTerminalMode();
#endif

public:
    static void initialize();
    static void cleanup();

    static void clear();

    static void setCursorPosition(int x, int y);

    static std::pair<int, int> getSize();

    static void setColor(TextColor fg, TextColor bg = TextColor::Default);

    static void setStyle(TextStyle style);

    static void resetAttributes();

    static void print(const std::string& text, TextColor fg = TextColor::Default,
                      TextColor bg = TextColor::Default, TextStyle style = TextStyle::Regular);

    static void println(const std::string& text, TextColor fg = TextColor::Default,
                        TextColor bg = TextColor::Default, TextStyle style = TextStyle::Regular);

    static std::string readLine();

    static char readChar();

    static bool keyPressed();

    static void drawBox(int x, int y, int width, int height,
                        TextColor fg = TextColor::Default, TextColor bg = TextColor::Default);

    static void drawHorizontalLine(int x, int y, int length,
                                  TextColor fg = TextColor::Default, TextColor bg = TextColor::Default);

    static void drawVerticalLine(int x, int y, int length,
                                TextColor fg = TextColor::Default, TextColor bg = TextColor::Default);

    static void drawTable(int x, int y, const std::vector<std::vector<std::string>>& data,
                          const std::vector<int>& columnWidths, bool hasHeader = true,
                          TextColor headerFg = TextColor::White, TextColor headerBg = TextColor::Blue,
                          TextColor bodyFg = TextColor::Default, TextColor bodyBg = TextColor::Default);

    static int showMenu(int x, int y, const std::vector<std::string>& options,
                       TextColor selectedFg = TextColor::Black, TextColor selectedBg = TextColor::White,
                       TextColor normalFg = TextColor::White, TextColor normalBg = TextColor::Default);

    static void drawChart(int x, int y, int width, int height, const std::vector<double>& values,
                         double minValue, double maxValue, TextColor fg = TextColor::Green);

    static void drawProgressBar(int x, int y, int width, double progress,
                               TextColor fg = TextColor::Green, TextColor bg = TextColor::Default);

    static void sleep(int milliseconds);

    static void waitForKey();

    template<typename Func>
    static void handleInput(Func callback) {
        char key;
        while (true) {
            if (keyPressed()) {
                key = readChar();
                if (!callback(key)) {
                    break;
                }
            }
            sleep(50);
        }
    }
};

}