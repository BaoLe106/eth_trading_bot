#include "risk.hpp"
#include <cmath>

static bool near_zero(double x) { return std::fabs(x) < 1e-12; }

bool allow_order(const RiskLimits& lim,
                 const RiskState& st,
                 Side side,
                 double size_eth,
                 double mark_price_usd,
                 std::int64_t now_ts,
                 std::string& reason) {
    // Cooldown
    if (lim.cooldown_seconds > 0 && st.last_fill_ts > 0) {
        if (now_ts - st.last_fill_ts < lim.cooldown_seconds) {
            reason = "cooldown active";
            return false;
        }
    }

    // Daily loss cap (very naive: uses realized only)
    if (st.realized_pnl <= -std::fabs(lim.max_daily_loss)) {
        reason = "daily loss limit reached";
        return false;
    }

    // Long-only rules
    if (lim.long_only) {
        if (side == Side::Sell) {
            if (size_eth > st.position_eth + 1e-12) {
                reason = "long-only: cannot sell more than current position";
                return false;
            }
        }
        double new_pos = st.position_eth + (side == Side::Buy ? size_eth : -size_eth);
        if (std::fabs(new_pos) > lim.max_position_abs + 1e-12) {
            reason = "max position exceeded";
            return false;
        }
        if (new_pos < -1e-12) {
            reason = "long-only: no short positions";
            return false;
        }
    } else {
        (void)mark_price_usd;
    }

    reason.clear();
    return true;
}

void on_fill(RiskState& st,
             Side side,
             double size_eth,
             double fill_price_usd,
             std::int64_t fill_ts) {
    if (side == Side::Buy) {
        double new_pos = st.position_eth + size_eth;
        if (near_zero(new_pos)) {
            st.avg_cost_usd = 0.0;
        } else if (st.position_eth <= 0.0) {
            st.avg_cost_usd = fill_price_usd;
        } else {
            st.avg_cost_usd = (st.avg_cost_usd * st.position_eth + fill_price_usd * size_eth) / new_pos;
        }
        st.position_eth = new_pos;
    } else { // Sell
        double sell_qty = std::min(size_eth, st.position_eth);
        st.realized_pnl += sell_qty * (fill_price_usd - st.avg_cost_usd);
        st.position_eth -= sell_qty;
        if (near_zero(st.position_eth)) {
            st.position_eth = 0.0;
            st.avg_cost_usd = 0.0;
        }
    }
    st.last_fill_ts = fill_ts;
}