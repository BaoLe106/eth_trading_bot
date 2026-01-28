#include "include/data_client.hpp"
#include "include/http.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

std::vector<Candle> fetch_candles_local(const std::string& base_url,
                                        const std::string& product,
                                        const std::string& granularity,
                                        int lookback_minutes) {
    std::string url = base_url + "/candles?product_id=" + product
                    + "&granularity=" + granularity
                    + "&lookback_minutes=" + std::to_string(lookback_minutes);
    auto s = http_get(url);
    auto j = nlohmann::json::parse(s);

    std::vector<Candle> out;
    for (auto& c : j.at("candles")) {
        Candle k{};
        k.start  = std::stol(c.at("start").get<std::string>());
        k.open   = std::stod(c.at("open").get<std::string>());
        k.high   = std::stod(c.at("high").get<std::string>());
        k.low    = std::stod(c.at("low").get<std::string>());
        k.close  = std::stod(c.at("close").get<std::string>());
        k.volume = std::stod(c.at("volume").get<std::string>());
        out.push_back(k);
    }
    return out;
}