#pragma once
#include <string>
#include <cstdint>

enum class Side { Buy, Sell };

struct RiskLimits {
    double max_position_abs = 0.05;
    double max_daily_loss   = 50.0;
    int    cooldown_seconds = 60;
    bool   long_only        = true;
};

struct RiskState {
    double position_eth   = 0.0;
    double avg_cost_usd   = 0.0;
    double realized_pnl   = 0.0;
    std::int64_t last_fill_ts = 0;
};

bool allow_order(const RiskLimits& lim,
                 const RiskState& st,
                 Side side,
                 double size_eth,
                 double mark_price_usd,
                 std::int64_t now_ts,
                 std::string& reason);

void on_fill(RiskState& st,
             Side side,
             double size_eth,
             double fill_price_usd,
             std::int64_t fill_ts);