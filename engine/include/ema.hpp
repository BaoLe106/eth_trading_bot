#pragma once
#include <cstddef>

class EMA {
public:
    explicit EMA(std::size_t period = 12);
    void reset();
    double update(double price);
    bool ready() const;
    double value() const;

private:
    std::size_t period_;
    double alpha_;
    bool initialized_;
    std::size_t seen_;
    double ema_;
};