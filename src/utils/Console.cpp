#include "Console.hpp"
#include <thread>
#include <chrono>
#include <cstdio>

#ifdef _WIN32
#include <conio.h>
#else
#include <fcntl.h>
#endif

namespace StockMarketSimulator {

TextColor Console::currentFgColor = TextColor::Default;
TextColor Console::currentBgColor = TextColor::Default;
TextStyle Console::currentStyle = TextStyle::Regular;
bool Console::isInitialized = false;

#ifndef _WIN32
struct termios Console::originalTermios;
bool Console::termiosSet = false;

void Console::setupTerminalMode() {
    if (!termiosSet) {
        tcgetattr(STDIN_FILENO, &originalTermios);
        termiosSet = true;

        struct termios raw = originalTermios;

        raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

        raw.c_iflag &= ~(IXON | ICRNL | INPCK | ISTRIP);

        raw.c_oflag &= ~(OPOST);

        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
}

void Console::restoreTerminalMode() {
    if (termiosSet) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
        termiosSet = false;
    }
}
#endif

std::string Console::getAnsiColorCode(TextColor fg, TextColor bg, TextStyle style) {
    std::string code = "\033[";

    switch (style) {
        case TextStyle::Bold: code += "1;"; break;
        case TextStyle::Underline: code += "4;"; break;
        default: code += "0;"; break;
    }

    if (fg != TextColor::Default) {
        code += std::to_string(30 + static_cast<int>(fg)) + ";";
    }

    if (bg != TextColor::Default) {
        code += std::to_string(40 + static_cast<int>(bg));
    } else {
        if (code.back() == ';') {
            code.pop_back();
        }
    }

    code += "m";
    return code;
}

void Console::initializeConsole() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    setupTerminalMode();
#endif
    isInitialized = true;
}

void Console::initialize() {
    if (!isInitialized) {
        initializeConsole();
    }
}

void Console::cleanup() {
#ifndef _WIN32
    restoreTerminalMode();
#endif
}

void Console::clear() {
    initialize();
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H" << std::flush;
#endif
}

void Console::setCursorPosition(int x, int y) {
    initialize();
    std::cout << "\033[" << y + 1 << ";" << x + 1 << "H" << std::flush;
}

std::pair<int, int> Console::getSize() {
    initialize();
    int width = 80;
    int height = 24;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        width = w.ws_col;
        height = w.ws_row;
    }
#endif

    return {width, height};
}

void Console::setColor(TextColor fg, TextColor bg) {
    initialize();
    currentFgColor = fg;
    currentBgColor = bg;
    std::cout << getAnsiColorCode(fg, bg, currentStyle) << std::flush;
}

void Console::setStyle(TextStyle style) {
    initialize();
    currentStyle = style;
    std::cout << getAnsiColorCode(currentFgColor, currentBgColor, style) << std::flush;
}

void Console::resetAttributes() {
    initialize();
    currentFgColor = TextColor::Default;
    currentBgColor = TextColor::Default;
    currentStyle = TextStyle::Regular;
    std::cout << "\033[0m" << std::flush;
}

void Console::print(const std::string& text, TextColor fg, TextColor bg, TextStyle style) {
    initialize();
    if (fg != currentFgColor || bg != currentBgColor || style != currentStyle) {
        std::cout << getAnsiColorCode(fg, bg, style);
    }
    std::cout << text << std::flush;
    if (fg != currentFgColor || bg != currentBgColor || style != currentStyle) {
        std::cout << getAnsiColorCode(currentFgColor, currentBgColor, currentStyle);
    }
}

void Console::println(const std::string& text, TextColor fg, TextColor bg, TextStyle style) {
    print(text, fg, bg, style);
    std::cout << std::endl;
}

std::string Console::readLine() {
    initialize();

#ifndef _WIN32
    restoreTerminalMode();
#endif

    std::string input;
    std::getline(std::cin, input);

#ifndef _WIN32
    setupTerminalMode();
#endif

    return input;
}

bool Console::keyPressed() {
    initialize();
#ifdef _WIN32
    return _kbhit() != 0;
#else
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

    return (result > 0);
#endif
}

char Console::readChar() {
    initialize();
#ifdef _WIN32
    return _getch();
#else
    char c = 0;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);

    if (read(STDIN_FILENO, &c, 1) < 0) {
        return 0;
    }

    if (c == 27) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

        if (result > 0) {
            char seq[2] = {0, 0};
            if (read(STDIN_FILENO, &seq[0], 1) == 1) {
                if (seq[0] == '[') {
                    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
                        if (read(STDIN_FILENO, &seq[1], 1) == 1) {
                            switch (seq[1]) {
                                case 'A': return static_cast<char>(Key::ArrowUp);
                                case 'B': return static_cast<char>(Key::ArrowDown);
                                case 'C': return static_cast<char>(Key::ArrowRight);
                                case 'D': return static_cast<char>(Key::ArrowLeft);
                            }
                        }
                    }
                }
            }
        }

        return static_cast<char>(Key::Escape);
    }

    if (c == 10) {
        return static_cast<char>(Key::Enter);
    }

    return c;
#endif
}

void Console::drawBox(int x, int y, int width, int height, TextColor fg, TextColor bg) {
    initialize();
    setColor(fg, bg);

    setCursorPosition(x, y);
    std::cout << "+" << std::string(width - 2, '-') << "+";

    for (int i = 1; i < height - 1; i++) {
        setCursorPosition(x, y + i);
        std::cout << "|";
        setCursorPosition(x + width - 1, y + i);
        std::cout << "|";
    }

    setCursorPosition(x, y + height - 1);
    std::cout << "+" << std::string(width - 2, '-') << "+";

    resetAttributes();
}

void Console::drawHorizontalLine(int x, int y, int length, TextColor fg, TextColor bg) {
    initialize();
    setColor(fg, bg);
    setCursorPosition(x, y);
    std::cout << "+" + std::string(length - 2, '-') + "+";
    resetAttributes();
}

void Console::drawVerticalLine(int x, int y, int length, TextColor fg, TextColor bg) {
    initialize();
    setColor(fg, bg);
    for (int i = 0; i < length; i++) {
        setCursorPosition(x, y + i);
        std::cout << "|";
    }
    resetAttributes();
}

void Console::drawTable(int x, int y, const std::vector<std::vector<std::string>>& data,
                        const std::vector<int>& columnWidths, bool hasHeader,
                        TextColor headerFg, TextColor headerBg,
                        TextColor bodyFg, TextColor bodyBg) {
    initialize();

    if (data.empty() || columnWidths.empty()) {
        return;
    }

    int totalWidth = 1;
    for (int width : columnWidths) {
        totalWidth += width + 1;
    }

    setCursorPosition(x, y);
    setColor(bodyFg, bodyBg);
    std::cout << "+";
    for (int width : columnWidths) {
        std::cout << std::string(width, '-') << "+";
    }

    int currentY = y + 1;

    for (size_t i = 0; i < data.size(); i++) {
        const auto& row = data[i];
        setCursorPosition(x, currentY);

        if (i == 0 && hasHeader) {
            setColor(headerFg, headerBg);
        } else {
            setColor(bodyFg, bodyBg);
        }

        std::cout << "|";

        for (size_t j = 0; j < columnWidths.size() && j < row.size(); j++) {
            std::string cell = row[j];
            if (cell.length() > static_cast<size_t>(columnWidths[j])) {
                cell = cell.substr(0, columnWidths[j] - 3) + "...";
            }
            std::cout << cell << std::string(columnWidths[j] - cell.length(), ' ') << "|";
        }

        currentY++;

        if ((i == 0 && hasHeader) || i < data.size() - 1) {
            setCursorPosition(x, currentY);
            setColor(bodyFg, bodyBg);
            std::cout << "+";
            for (int width : columnWidths) {
                std::cout << std::string(width, '-') << "+";
            }
            currentY++;
        }
    }

    setCursorPosition(x, currentY);
    setColor(bodyFg, bodyBg);
    std::cout << "+";
    for (int width : columnWidths) {
        std::cout << std::string(width, '-') << "+";
    }

    resetAttributes();
}

int Console::showMenu(int x, int y, const std::vector<std::string>& options,
                     TextColor selectedFg, TextColor selectedBg,
                     TextColor normalFg, TextColor normalBg) {
    initialize();

    if (options.empty()) {
        return -1;
    }

    size_t maxWidth = 0;
    for (const auto& option : options) {
        maxWidth = std::max(maxWidth, option.length());
    }

    int menuWidth = static_cast<int>(maxWidth) + 4;
    int selected = 0;
    bool running = true;
    std::string buffer;

    while (running) {
        buffer.clear();

        for (size_t i = 0; i < options.size(); i++) {
            buffer += "\033[" + std::to_string(y + static_cast<int>(i) + 1) + ";" + std::to_string(x + 1) + "H";

            if (i == static_cast<size_t>(selected)) {
                buffer += getAnsiColorCode(selectedFg, selectedBg, TextStyle::Regular);
                buffer += "> " + options[i];
                buffer += std::string(menuWidth - options[i].length() - 2, ' ');
            } else {
                // Normal item formatting
                buffer += getAnsiColorCode(normalFg, normalBg, TextStyle::Regular);
                buffer += "  " + options[i];
                buffer += std::string(menuWidth - options[i].length() - 2, ' ');
            }
        }

        buffer += getAnsiColorCode(TextColor::Default, TextColor::Default, TextStyle::Regular);

        std::cout << buffer << std::flush;

        char key = readChar();

        switch (key) {
            case static_cast<char>(Key::ArrowUp):
                selected = (selected > 0) ? selected - 1 : static_cast<int>(options.size()) - 1;
                break;

            case static_cast<char>(Key::ArrowDown):
                selected = (selected < static_cast<int>(options.size()) - 1) ? selected + 1 : 0;
                break;

            case static_cast<char>(Key::Enter):
                running = false;
                break;

            case static_cast<char>(Key::Escape):
                return -1;

            default:
                if (key >= '0' && key <= '9') {
                    int index = key - '0';
                    if (index == 0) index = 10;
                    index--;

                    if (index >= 0 && index < static_cast<int>(options.size())) {
                        return index;
                    }
                }
                break;
        }
    }

    return selected;
}

void Console::drawChart(int x, int y, int width, int height, const std::vector<double>& values,
                      double minValue, double maxValue, TextColor fg) {
    initialize();

    if (values.empty() || width <= 2 || height <= 2) {
        return;
    }

    if (minValue == maxValue) {
        minValue = values[0];
        maxValue = values[0];

        for (const double& value : values) {
            if (value < minValue) minValue = value;
            if (value > maxValue) maxValue = value;
        }

        double range = maxValue - minValue;
        if (range == 0) {
            minValue -= 1.0;
            maxValue += 1.0;
        } else {
            minValue -= range * 0.05;
            maxValue += range * 0.05;
        }
    }

    double valueRange = maxValue - minValue;
    if (valueRange == 0) valueRange = 1.0;

    drawBox(x, y, width, height, fg, TextColor::Default);

    int dataWidth = width - 2;
    int dataHeight = height - 2;

    int numPoints = static_cast<int>(values.size());

    setColor(fg, TextColor::Default);

    for (int i = 0; i < dataWidth; i++) {
        double dataIndex = (numPoints <= 1) ? 0 : (i * (numPoints - 1.0)) / std::max(1.0, (dataWidth - 1.0));
        int lowerIndex = static_cast<int>(dataIndex);
        int upperIndex = std::min(lowerIndex + 1, numPoints - 1);
        double fraction = dataIndex - lowerIndex;

        double value;
        if (lowerIndex == upperIndex || numPoints <= 1) {
            value = values[lowerIndex];
        } else {
            value = values[lowerIndex] * (1.0 - fraction) + values[upperIndex] * fraction;
        }

        double normalizedValue = (value - minValue) / valueRange;
        int pointHeight = static_cast<int>(normalizedValue * dataHeight);
        if (pointHeight < 0) pointHeight = 0;
        if (pointHeight > dataHeight) pointHeight = dataHeight;

        int pointX = x + 1 + i;

        for (int j = 0; j < pointHeight; j++) {
            setCursorPosition(pointX, y + dataHeight - j);
            std::cout << "█";
        }
    }

    resetAttributes();
}
void Console::drawProgressBar(int x, int y, int width, double progress,
                            TextColor fg, TextColor bg) {
    initialize();

    if (width < 2) return;

    if (progress < 0.0) progress = 0.0;
    if (progress > 1.0) progress = 1.0;

    int progressWidth = static_cast<int>(progress * (width - 2));

    setCursorPosition(x, y);
    setColor(fg, bg);

    std::cout << "[";

    for (int i = 0; i < width - 2; i++) {
        if (i < progressWidth) {
            std::cout << "█";
        } else {
            std::cout << " ";
        }
    }

    std::cout << "]";

    resetAttributes();
}

void Console::sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Console::waitForKey() {
    initialize();
    std::cout << "Press any key to continue..." << std::flush;
    readChar();
}

}