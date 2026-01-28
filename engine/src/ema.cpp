#include "include/ema.hpp"
#include <algorithm>

EMA::EMA(std::size_t period)
: period_(std::max<std::size_t>(1, period)),
  alpha_(2.0 / (period_ + 1.0)),
  initialized_(false),
  seen_(0),
  ema_(0.0) {}

void EMA::reset() { initialized_ = false; seen_ = 0; ema_ = 0.0; }

double EMA::update(double price) {
    if (!initialized_) { ema_ = price; initialized_ = true; }
    else { ema_ = alpha_ * price + (1.0 - alpha_) * ema_; }
    ++seen_;
    return ema_;
}

bool EMA::ready() const { return seen_ >= period_; }
double EMA::value() const { return ema_; }