#include "Chart.hpp"
#include <algorithm>

namespace StockMarketSimulator {

Chart::Chart()
    : x(0), y(0), width(40), height(10), minValue(0.0), maxValue(0.0),
      color(TextColor::Green), autoScale(true), title(""), yLabel("")
{
}

Chart::Chart(int x, int y, int width, int height, const std::string& title)
    : x(x), y(y), width(width), height(height), minValue(0.0), maxValue(0.0),
      color(TextColor::Green), autoScale(true), title(title), yLabel("")
{
}

void Chart::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
}

void Chart::setSize(int width, int height) {
    if (width > 0 && height > 0) {
        this->width = width;
        this->height = height;
    }
}

void Chart::setTitle(const std::string& title) {
    this->title = title;
}

void Chart::setColor(TextColor color) {
    this->color = color;
}

void Chart::setAutoScale(bool autoScale) {
    this->autoScale = autoScale;
}

void Chart::setData(const std::vector<double>& data) {
    this->data = data;
    if (autoScale) {
        calculateMinMax();
    }
}

void Chart::addDataPoint(double value) {
    data.push_back(value);
    if (autoScale) {
        calculateMinMax();
    }
}

void Chart::clearData() {
    data.clear();
    minValue = 0.0;
    maxValue = 0.0;
}

void Chart::setXLabels(const std::vector<std::string>& labels) {
    xLabels = labels;
}

void Chart::setYLabel(const std::string& label) {
    yLabel = label;
}

const std::vector<double>& Chart::getData() const {
    return data;
}

std::string Chart::getTitle() const {
    return title;
}

int Chart::getX() const {
    return x;
}

int Chart::getY() const {
    return y;
}

int Chart::getWidth() const {
    return width;
}

int Chart::getHeight() const {
    return height;
}

TextColor Chart::getColor() const {
    return color;
}

bool Chart::getAutoScale() const {
    return autoScale;
}

const std::vector<std::string>& Chart::getXLabels() const {
    return xLabels;
}

std::string Chart::getYLabel() const {
    return yLabel;
}

void Chart::calculateMinMax() {
    if (data.empty()) {
        minValue = 0.0;
        maxValue = 0.0;
        return;
    }

    minValue = *std::min_element(data.begin(), data.end());
    maxValue = *std::max_element(data.begin(), data.end());

    double range = maxValue - minValue;
    if (range == 0) {
        minValue -= 1.0;
        maxValue += 1.0;
    } else {
        minValue -= range * 0.05;
        maxValue += range * 0.05;
    }
}

void Chart::draw() const {
    int chartY = y;
    int chartHeight = height;

    // Draw title if present
    if (!title.empty()) {
        Console::setCursorPosition(x, chartY);
        Console::print(title, color);
        chartY += 1;
        chartHeight -= 1;
    }

    // Draw chart with box
    Console::drawChart(x, chartY, width, chartHeight, data, minValue, maxValue, color);

    // Draw X labels if present
    if (!xLabels.empty()) {
        int labelY = chartY + chartHeight;
        int numLabels = std::min(static_cast<int>(xLabels.size()), width - 2);
        int labelWidth = (width - 2) / numLabels;

        for (int i = 0; i < numLabels; i++) {
            int labelX = x + 1 + i * labelWidth;
            Console::setCursorPosition(labelX, labelY);

            // Truncate label if too long
            std::string label = xLabels[i];
            if (label.length() > static_cast<size_t>(labelWidth)) {
                label = label.substr(0, labelWidth - 1);
            }

            Console::print(label, color);
        }
    }

    // Draw Y label if present
    if (!yLabel.empty()) {
        // Draw Y label vertically along the left side of the chart
        for (size_t i = 0; i < yLabel.length() && i < static_cast<size_t>(chartHeight); i++) {
            Console::setCursorPosition(x - 2, chartY + i);
            std::string c(1, yLabel[i]);
            Console::print(c, color);
        }
    }
}

}