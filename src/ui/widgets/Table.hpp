#pragma once

#include <string>
#include <vector>
#include <functional>
#include "../../utils/Console.hpp"

namespace StockMarketSimulator {

class Table {
private:
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> data;
    std::vector<int> columnWidths;
    int x, y;
    bool hasHeader;
    TextColor headerFg, headerBg;
    TextColor bodyFg, bodyBg;
    bool visible;
    std::function<bool(const std::vector<std::string>&, const std::vector<std::string>&, int)> sortFunction;

    static double parseNumberFromString(const std::string& str);

public:
    Table();
    Table(int x, int y, bool hasHeader = true);
    Table(int x, int y, const std::vector<std::string>& headers,
          const std::vector<int>& columnWidths);

    void setPosition(int x, int y);
    int getX() const;
    int getY() const;

    void setHeaders(const std::vector<std::string>& headers);
    const std::vector<std::string>& getHeaders() const;

    void setColumnWidths(const std::vector<int>& widths);
    const std::vector<int>& getColumnWidths() const;

    void setHeaderColors(TextColor fg, TextColor bg);
    void setBodyColors(TextColor fg, TextColor bg);

    TextColor getHeaderFg() const;
    TextColor getHeaderBg() const;
    TextColor getBodyFg() const;
    TextColor getBodyBg() const;

    void setHasHeader(bool hasHeader);
    bool getHasHeader() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void clearData();
    void addRow(const std::vector<std::string>& row);
    void removeRow(int index);
    void updateRow(int index, const std::vector<std::string>& newRow);
    const std::vector<std::vector<std::string>>& getData() const;

    void setSortFunction(std::function<bool(const std::vector<std::string>&,
                         const std::vector<std::string>&, int)> sortFunc);
    void sortByColumn(int columnIndex, bool ascending = true);

    void draw() const;

    int calculateTableHeight() const;
    int calculateTableWidth() const;
};

}