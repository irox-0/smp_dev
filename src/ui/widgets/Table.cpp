#include "Table.hpp"
#include <algorithm>

namespace StockMarketSimulator {

double Table::parseNumberFromString(const std::string& str) {
    double value = 0.0;
    try {
        size_t start = 0;
        while (start < str.length() && !isdigit(str[start]) && str[start] != '-' && str[start] != '+') {
            start++;
        }

        if (start >= str.length()) {
            return 0.0;
        }

        size_t end = start;
        while (end < str.length() && (isdigit(str[end]) || str[end] == '.' || str[end] == ',' ||
               (end == start && (str[end] == '-' || str[end] == '+')))) {
            end++;
        }

        std::string numStr = str.substr(start, end - start);
        std::replace(numStr.begin(), numStr.end(), ',', '.');

        value = std::stod(numStr);
    } catch (...) {
    }
    return value;
}

Table::Table()
    : x(0), y(0), hasHeader(true),
      headerFg(TextColor::White), headerBg(TextColor::Blue),
      bodyFg(TextColor::Default), bodyBg(TextColor::Default),
      visible(true)
{
}

Table::Table(int x, int y, bool hasHeader)
    : x(x), y(y), hasHeader(hasHeader),
      headerFg(TextColor::White), headerBg(TextColor::Blue),
      bodyFg(TextColor::Default), bodyBg(TextColor::Default),
      visible(true)
{
}

Table::Table(int x, int y, const std::vector<std::string>& headers,
           const std::vector<int>& columnWidths)
    : x(x), y(y), headers(headers), columnWidths(columnWidths), hasHeader(true),
      headerFg(TextColor::White), headerBg(TextColor::Blue),
      bodyFg(TextColor::Default), bodyBg(TextColor::Default),
      visible(true)
{
    if (columnWidths.size() != headers.size()) {
        this->columnWidths.clear();
        for (const auto& header : headers) {
            this->columnWidths.push_back(header.length() + 2);
        }
    }
}

void Table::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

int Table::getX() const {
    return x;
}

int Table::getY() const {
    return y;
}

void Table::setHeaders(const std::vector<std::string>& headers) {
    this->headers = headers;

    if (columnWidths.size() != headers.size()) {
        columnWidths.clear();
        for (const auto& header : headers) {
            columnWidths.push_back(header.length() + 2);
        }
    }
}

const std::vector<std::string>& Table::getHeaders() const {
    return headers;
}

void Table::setColumnWidths(const std::vector<int>& widths) {
    columnWidths = widths;
}

const std::vector<int>& Table::getColumnWidths() const {
    return columnWidths;
}

void Table::setHeaderColors(TextColor fg, TextColor bg) {
    headerFg = fg;
    headerBg = bg;
}

void Table::setBodyColors(TextColor fg, TextColor bg) {
    bodyFg = fg;
    bodyBg = bg;
}

TextColor Table::getHeaderFg() const {
    return headerFg;
}

TextColor Table::getHeaderBg() const {
    return headerBg;
}

TextColor Table::getBodyFg() const {
    return bodyFg;
}

TextColor Table::getBodyBg() const {
    return bodyBg;
}

void Table::setHasHeader(bool hasHeader) {
    this->hasHeader = hasHeader;
}

bool Table::getHasHeader() const {
    return hasHeader;
}

void Table::setVisible(bool visible) {
    this->visible = visible;
}

bool Table::isVisible() const {
    return visible;
}

void Table::clearData() {
    data.clear();
}

void Table::addRow(const std::vector<std::string>& row) {
    if (headers.empty() || row.size() == headers.size()) {
        data.push_back(row);
    } else {
        std::vector<std::string> adjustedRow = row;
        adjustedRow.resize(headers.size(), "");
        data.push_back(adjustedRow);
    }
}

void Table::removeRow(int index) {
    if (index >= 0 && index < static_cast<int>(data.size())) {
        data.erase(data.begin() + index);
    }
}

void Table::updateRow(int index, const std::vector<std::string>& newRow) {
    if (index >= 0 && index < static_cast<int>(data.size())) {
        if (headers.empty() || newRow.size() == headers.size()) {
            data[index] = newRow;
        } else {
            std::vector<std::string> adjustedRow = newRow;
            adjustedRow.resize(headers.size(), "");
            data[index] = adjustedRow;
        }
    }
}

const std::vector<std::vector<std::string>>& Table::getData() const {
    return data;
}

void Table::setSortFunction(std::function<bool(const std::vector<std::string>&,
                           const std::vector<std::string>&, int)> sortFunc) {
    sortFunction = sortFunc;
}

void Table::sortByColumn(int columnIndex, bool ascending) {
    if (columnIndex < 0 || columnIndex >= static_cast<int>(headers.size()) || data.empty()) {
        return;
    }

    if (sortFunction) {
        std::sort(data.begin(), data.end(),
            [this, columnIndex, ascending](const auto& a, const auto& b) {
                bool result = sortFunction(a, b, columnIndex);
                return ascending ? result : !result;
            });
    } else {
        bool isNumeric = false;
        if (!data.empty() && columnIndex < static_cast<int>(data[0].size())) {
            int samplesToCheck = std::min(5, static_cast<int>(data.size()));
            for (int i = 0; i < samplesToCheck; i++) {
                if (columnIndex < static_cast<int>(data[i].size())) {
                    try {
                        parseNumberFromString(data[i][columnIndex]);
                        isNumeric = true;
                        break;
                    } catch (...) {
                    }
                }
            }
        }

        if (isNumeric) {
            std::sort(data.begin(), data.end(),
                [columnIndex, ascending](const auto& a, const auto& b) {
                    if (columnIndex >= static_cast<int>(a.size()) ||
                        columnIndex >= static_cast<int>(b.size())) {
                        return false;
                    }

                    double valueA = parseNumberFromString(a[columnIndex]);
                    double valueB = parseNumberFromString(b[columnIndex]);

                    bool result = valueA < valueB;
                    return ascending ? result : !result;
                });
        } else {
            std::sort(data.begin(), data.end(),
                [columnIndex, ascending](const auto& a, const auto& b) {
                    if (columnIndex >= static_cast<int>(a.size()) ||
                        columnIndex >= static_cast<int>(b.size())) {
                        return false;
                    }

                    bool result = a[columnIndex] < b[columnIndex];
                    return ascending ? result : !result;
                });
        }
    }
}

void Table::draw() const {
    if (!visible) return;

    if (headers.empty() && data.empty()) return;

    int numColumns = !headers.empty() ? headers.size() :
                    (!data.empty() ? data[0].size() : 0);

    if (numColumns == 0) return;

    std::vector<std::vector<std::string>> displayData;

    if (hasHeader && !headers.empty()) {
        displayData.push_back(headers);
    }

    for (const auto& row : data) {
        displayData.push_back(row);
    }

    std::vector<int> actualColumnWidths = columnWidths;
    if (actualColumnWidths.empty() || actualColumnWidths.size() != numColumns) {
        actualColumnWidths.resize(numColumns, 10);

        for (const auto& row : displayData) {
            for (size_t i = 0; i < row.size() && i < numColumns; ++i) {
                actualColumnWidths[i] = std::max(actualColumnWidths[i],
                                              static_cast<int>(row[i].length()) + 2);
            }
        }
    }

    Console::drawTable(x, y, displayData, actualColumnWidths, hasHeader,
                     headerFg, headerBg, bodyFg, bodyBg);
}

int Table::calculateTableHeight() const {

    int numDataRows = static_cast<int>(data.size());

    // Calculate the table's height (accounting for all rows, separators, and borders)
    int tableHeight;
    if (!headers.empty()) {
        tableHeight = 3 + (2 * numDataRows); // Top border, header, separator, rows with separators, bottom border
    } else {
        tableHeight = 1 + (2 * numDataRows); // Top border, rows with separators, bottom border
    }
    //int rowCount = static_cast<int>(data.size());
    return tableHeight;
}

int Table::calculateTableWidth() const {
    int width = 1;
    for (int columnWidth : columnWidths) {
        width += columnWidth + 1;
    }
    return width;
}

}