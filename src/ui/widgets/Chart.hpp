#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../../utils/Console.hpp"

namespace StockMarketSimulator {

class Chart {
private:
    std::vector<double> data;
    std::string title;
    int x, y, width, height;
    double minValue, maxValue;
    TextColor color;
    bool autoScale;
    std::vector<std::string> xLabels;
    std::string yLabel;

    void calculateMinMax();

public:
    Chart();
    Chart(int x, int y, int width, int height, const std::string& title = "");

    void setPosition(int x, int y);
    void setSize(int width, int height);
    void setTitle(const std::string& title);
    void setColor(TextColor color);
    void setAutoScale(bool autoScale);
    void setData(const std::vector<double>& data);
    void addDataPoint(double value);
    void clearData();
    void setXLabels(const std::vector<std::string>& labels);
    void setYLabel(const std::string& label);

    const std::vector<double>& getData() const;
    std::string getTitle() const;
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    TextColor getColor() const;
    bool getAutoScale() const;
    const std::vector<std::string>& getXLabels() const;
    std::string getYLabel() const;

    void draw() const;
};

}