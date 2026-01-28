# Ethereum Trading Bot — Split Starter (C++ engine + Python data API)

**Goal**: 
- **C++ engine** computes EMA(12/26) crossover and can **place BUY/SELL** (sandbox/prod wiring left as a TODO).
- **Python FastAPI** exposes **/candles** and **/ticker** by fetching Coinbase public market data.

> This keeps Python focused on **data only**, and C++ in charge of **strategy + orders**.

---

## 1) Python data API

### Setup
```bash
cd broker
python -m venv .venv && source .venv/bin/activate
pip install fastapi uvicorn httpx pydantic
uvicorn app:app --reload --port 8000
```

### Endpoints
- `GET /candles?product_id=ETH-USD&granularity=ONE_MINUTE&lookback_minutes=300`
- `GET /ticker?product_id=ETH-USD`

These call Coinbase **public** market endpoints (no auth).

---

## 2) C++ engine

### Build
```bash
cd engine
cmake -S . -B build
cmake --build build --config Release
```

### Run (dry-run, no orders)
```bash
./build/ethbot --lookback 300 --fast 12 --slow 26 --size 0.01 --dry-run true
```

### Run (attempt orders — requires wiring in order_placer.cpp TODOs)
```bash
./build/ethbot --lookback 300 --fast 12 --slow 26 --size 0.01 --sandbox true --dry-run false
```

**Flags**
- `--lookback N` minutes of candles to fetch
- `--fast / --slow` EMA periods (require 0<fast<slow)
- `--size` base size in ETH (string like 0.01)
- `--sandbox true|false` (if false → prod; order code requires JWT header wiring)
- `--dry-run true|false` do not hit order endpoint when true

---

## 3) Order placement notes (important)

- This starter leaves **actual Coinbase order signing** as a **TODO** in `engine/src/order_placer.cpp` so you can implement it yourself for interviews.
- Suggested approaches:
  - **Sandbox**: Some endpoints may still require auth; treat as **real auth path** (safer). Keep order flow behind `--dry-run` until you implement JWT/HMAC correctly.
  - **Prod**: Get a short-lived JWT/Bearer token from a helper (you can extend the Python API later) and send `Authorization: Bearer <token>`.
- Always start with **paper logic** and **dry-run**, then toggle carefully.

---

## 4) Folder layout

```
ethbot-split-starter/
├─ broker/
│  └─ app.py
├─ engine/
│  ├─ include/
│  │  ├─ ema.hpp
│  │  ├─ strategy.hpp
│  │  ├─ http.hpp
│  │  ├─ data_client.hpp
│  │  └─ order_placer.hpp
│  ├─ src/
│  │  ├─ ema.cpp
│  │  ├─ strategy.cpp
│  │  ├─ http.cpp
│  │  ├─ data_client.cpp
│  │  ├─ order_placer.cpp
│  │  └─ main.cpp
│  └─ CMakeLists.txt
└─ README.md
```

---

## 5) What to implement next
- Replace the TODO in `order_placer.cpp` with **real** Coinbase Advanced Trade order creation (JWT/HMAC, headers, etc.).
- Add SQLite persistence, risk guardrails, and a WebSocket feed.