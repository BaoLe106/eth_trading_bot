#pragma once
#include <string>

enum class Signal { HOLD, BUY, SELL };

struct CrossState {
    bool have_prev = false;
    double prev_diff = 0.0;
};

inline Signal crossover_signal(CrossState& st, double fast, double slow) {
    double diff = fast - slow;
    Signal s = Signal::HOLD;
    if (st.have_prev) {
        if (st.prev_diff <= 0.0 && diff > 0.0) s = Signal::BUY;
        else if (st.prev_diff >= 0.0 && diff < 0.0) s = Signal::SELL;
    }
    st.have_prev = true;
    st.prev_diff = diff;
    return s;
}

inline const char* to_str(Signal s) {
    switch (s) {
        case Signal::BUY: return "BUY";
        case Signal::SELL: return "SELL";
        default: return "HOLD";
    }
}