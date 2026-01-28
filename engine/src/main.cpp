#include "include/ema.hpp"
#include "include/strategy.hpp"
#include "include/data_client.hpp"
#include "include/order_placer.hpp"
#include <iostream>
#include <string>
#include "include/risk.hpp"
#include "include/logger.hpp"

struct Config {
    std::string data_api = "http://localhost:8000";
    std::string product = "ETH-USD";
    std::string granularity = "ONE_MINUTE";
    int lookback = 300;
    int fast = 12;
    int slow = 26;
    std::string size = "0.01";
    bool sandbox = true;
    bool dry_run = true;
};

Config parse_args(int argc, char** argv) {

    Config c;
    for (int i=1; i<argc; ++i) {
        std::string a = argv[i];
        auto next = [&](int& i)->std::string{ if(i+1<argc) return std::string(argv[++i]); throw std::runtime_error("Missing value for "+a); };
        if (a=="--api") c.data_api = next(i);
        else if (a=="--product") c.product = next(i);
        else if (a=="--granularity") c.granularity = next(i);
        else if (a=="--lookback") c.lookback = std::stoi(next(i));
        else if (a=="--fast") c.fast = std::stoi(next(i));
        else if (a=="--slow") c.slow = std::stoi(next(i));
        else if (a=="--size") c.size = next(i);
        else if (a=="--sandbox") c.sandbox = (next(i)=="true");
        else if (a=="--dry-run") c.dry_run = (next(i)=="true");
        else if (a=="-h" || a=="--help") {
            std::cout << "Usage: ethbot [--api URL] [--product ETH-USD] [--granularity ONE_MINUTE]\n"
                         "              [--lookback 300] [--fast 12] [--slow 26] [--size 0.01]\n"
                         "              [--sandbox true|false] [--dry-run true|false]\n";
            std::exit(0);
        }
    }
    if (c.fast<=0 || c.slow<=0 || c.fast>=c.slow) throw std::runtime_error("Require 0 < fast < slow");
    return c;
}

int main(int argc, char** argv) {
    try {
        Config cfg = parse_args(argc, argv);
        auto candles = fetch_candles_local(cfg.data_api, cfg.product, cfg.granularity, cfg.lookback);
        std::cout << "Fetched candles: " << candles.size() << "\n";

        EMA ema_f(cfg.fast), ema_s(cfg.slow);
        CrossState cs;

        logger_init("logs/strategy_log.csv");

        RiskLimits limits;
        limits.max_position_abs = 0.05;
        limits.max_daily_loss   = 50.0;
        limits.cooldown_seconds = 60;
        limits.long_only        = true;

        RiskState rstate;

        for (auto& k : candles) {
            double f = ema_f.update(k.close);
            double s = ema_s.update(k.close);
            if (!(ema_f.ready() && ema_s.ready())) {
                std::cout << k.start << " price=" << k.close << " warming up\n";
                continue;
            }
            auto sig = crossover_signal(cs, f, s);
            auto sigstr = sigstr_from_signal(sig);

            // default: no acion
            std::string action = "NONE";
            std::string reason;
            if (sig == Signal::BUY || sig == Signal::SELL) {
                Side side = side_from_signal(sig);

                bool ok = allow_order(limits, rstate, side, std::stod(cfg.size), k.close, k.start, reason);
                if (!ok) {
                    action = "BLOCKED";
                    logger_row(k.start, k.close, f, s, sigstr, action, rstate.position_eth, rstate.realized_pnl, reason);
                    std::cout << "BLOCKED: " << reason << "\n";
                } else if (cfg.dry_run) {
                    action = (sig == Signal::BUY ? "BUY" : "SELL");
                    on_fill(rstate, side, std::stod(cfg.size), k.close, k.start);
                    logger_row(k.start, k.close, f, s, sigstr, action, rstate.position_eth, rstate.realized_pnl, "");
                    std::cout << "DRY-RUN FILLED " << action << " " << cfg.size
                            << " @ " << k.close << " pos=" << rstate.position_eth
                            << " pnl=" << rstate.realized_pnl << "\n";
                } else {
                    action = (sig == Signal::BUY ? "BUY" : "SELL");
                    std::string bearer;
                    auto j = place_market_order(cfg.sandbox, action, cfg.size, bearer);
                    on_fill(rstate, side, std::stod(cfg.size), k.close, k.start);
                    logger_row(k.start, k.close, f, s, sigstr, action, rstate.position_eth, rstate.realized_pnl, "");
                    std::cout << "LIVE order response: " << j.dump() << "\n";
                }
            } else {
                logger_row(k.start, k.close, f, s, sigstr, action, rstate.position_eth, rstate.realized_pnl, "");
            }
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

static SigStr sigstr_from_signal(Signal s) {
    if (s == Signal::BUY)  return SigStr::Buy;
    if (s == Signal::SELL) return SigStr::Sell;
    return SigStr::Hold;
}
static Side side_from_signal(Signal s) {
    return (s == Signal::SELL) ? Side::Sell : Side::Buy;
}