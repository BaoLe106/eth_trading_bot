#pragma once
#include <string>
#include <cstdint>

enum class SigStr { Hold, Buy, Sell };

void logger_init(const std::string& path = "logs/strategy_log.csv");

void logger_row(std::int64_t ts,
                double price,
                double ema_fast,
                double ema_slow,
                SigStr signal,
                const std::string& action,
                double position_eth,
                double realized_pnl,
                const std::string& reason);