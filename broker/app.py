from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from typing import Optional
from datetime import datetime, timedelta, timezone
import httpx
import logging

app = FastAPI(title="ETH Data API (Coinbase public market)")

PUBLIC_BASE = "https://api.coinbase.com/api/v3/brokerage/market"

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class CandleResp(BaseModel):
    candles: list

@app.get("/candles", response_model=CandleResp)
async def candles(
    product_id: str = "ETH-USD",
    granularity: str = "ONE_MINUTE",
    lookback_minutes: int = 300
):
    try:
        end = datetime.now(timezone.utc)
        start = end - timedelta(minutes=lookback_minutes)
        url = f"{PUBLIC_BASE}/products/{product_id}/candles"
        params = {
            "start": int(start.timestamp()),
            "end": int(end.timestamp()),
            "granularity": granularity,
            "limit": 350,
        }
        async with httpx.AsyncClient(timeout=15) as client:
            r = await client.get(url, params=params, headers={"cache-control": "no-cache"})
            r.raise_for_status()
            return r.json()
    except httpx.TimeoutException:
        logger.error(f"Timeout fetching candles for {product_id}")
        raise HTTPException(status_code=504, detail="Request to Coinbase API timed out")
    except httpx.HTTPStatusError as e:
        logger.error(f"HTTP error fetching candles: {e.response.status_code}")
        raise HTTPException(status_code=e.response.status_code, detail=f"Coinbase API error: {str(e)}")
    except Exception as e:
        logger.error(f"Unexpected error fetching candles: {str(e)}")
        raise HTTPException(status_code=500, detail="Internal server error")

@app.get("/ticker")
async def ticker(product_id: str = "ETH-USD"):
    try:
        url = f"{PUBLIC_BASE}/products/{product_id}/ticker"
        async with httpx.AsyncClient(timeout=10) as client:
            r = await client.get(url, headers={"cache-control": "no-cache"})
            r.raise_for_status()
            return r.json()
    except httpx.TimeoutException:
        logger.error(f"Timeout fetching ticker for {product_id}")
        raise HTTPException(status_code=504, detail="Request to Coinbase API timed out")
    except httpx.HTTPStatusError as e:
        logger.error(f"HTTP error fetching ticker: {e.response.status_code}")
        raise HTTPException(status_code=e.response.status_code, detail=f"Coinbase API error: {str(e)}")
    except Exception as e:
        logger.error(f"Unexpected error fetching ticker: {str(e)}")
        raise HTTPException(status_code=500, detail="Internal server error")