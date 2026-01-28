#pragma once
#include <vector>
#include <string>

struct Candle {
    long start;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

std::vector<Candle> fetch_candles_local(const std::string& base_url,
                                        const std::string& product = "ETH-USD",
                                        const std::string& granularity = "ONE_MINUTE",
                                        int lookback_minutes = 300);