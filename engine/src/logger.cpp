#include "include/logger.hpp"
#include <fstream>
#include <filesystem>
#include <mutex>

static std::mutex g_mtx;
static bool g_inited = false;
static std::string g_path;

static const char* sig_to_cstr(SigStr s) {
    switch (s) {
        case SigStr::Buy:  return "BUY";
        case SigStr::Sell: return "SELL";
        default:           return "HOLD";
    }
}

void logger_init(const std::string& path) {
    std::scoped_lock lk(g_mtx);
    if (g_inited) return;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::ofstream f(path, std::ios::app);
    if (f.tellp() == 0) {
        f << "ts,price,ema_fast,ema_slow,signal,action,position_eth,realized_pnl,reason\n";
    }
    g_path = path;
    g_inited = true;
}

void logger_row(std::int64_t ts,
                double price,
                double ema_fast,
                double ema_slow,
                SigStr signal,
                const std::string& action,
                double position_eth,
                double realized_pnl,
                const std::string& reason)
{
    std::scoped_lock lk(g_mtx);
    if (!g_inited) logger_init();
    std::ofstream f(g_path, std::ios::app);
    f << ts << "," << price << "," << ema_fast << "," << ema_slow << ","
      << sig_to_cstr(signal) << "," << action << ","
      << position_eth << "," << realized_pnl << ","
      << '"' << reason << '"' << "\n";
}