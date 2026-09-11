from pathlib import Path
from datetime import datetime, timezone

import pandas as pd
import yfinance as yf


TICKERS = ["KO", "PEP"]
START_DATE = "2014-01-01"


def validate_download(ticker: str, df: pd.DataFrame) -> None:
    """Validate the downloaded Yahoo Finance dataset."""

    if df.empty:
        raise RuntimeError(
            f"{ticker}: Yahoo Finance returned no data."
        )

    required_columns = [
        "Open",
        "High",
        "Low",
        "Close",
        "Adj Close",
        "Volume",
    ]

    missing_columns = [
        column
        for column in required_columns
        if column not in df.columns
    ]

    if missing_columns:
        raise RuntimeError(
            f"{ticker}: missing columns: {missing_columns}"
        )

    if not df.index.is_monotonic_increasing:
        raise RuntimeError(
            f"{ticker}: timestamps are not monotonically increasing."
        )

    if df.index.has_duplicates:
        raise RuntimeError(
            f"{ticker}: duplicate timestamps detected."
        )

    # Every required field must be finite.
    if df[required_columns].isna().any().any():
        missing = df[required_columns].isna().sum()
        raise RuntimeError(
            f"{ticker}: missing values detected:\n{missing}"
        )

    # Prices must be positive.
    price_columns = [
        "Open",
        "High",
        "Low",
        "Close",
        "Adj Close",
    ]

    for column in price_columns:
        if (df[column] <= 0).any():
            raise RuntimeError(
                f"{ticker}: non-positive values in {column}."
            )

    # Volume cannot be negative.
    if (df["Volume"] < 0).any():
        raise RuntimeError(
            f"{ticker}: negative volume detected."
        )

    # OHLC consistency.
    inconsistent = (
        (df["High"] < df["Open"])
        | (df["High"] < df["Close"])
        | (df["High"] < df["Low"])
        | (df["Low"] > df["Open"])
        | (df["Low"] > df["Close"])
    )

    if inconsistent.any():
        count = int(inconsistent.sum())

        raise RuntimeError(
            f"{ticker}: {count} OHLC consistency violations."
        )


def download_ticker(
    ticker: str,
    start_date: str,
    output_dir: Path,
) -> None:

    print(
        f"\nFetching {ticker} "
        f"from {start_date}..."
    )

    df = yf.download(
        ticker,
        start=start_date,
        auto_adjust=False,
        actions=True,
        progress=False,
    )

    # yfinance can return a MultiIndex even for a single ticker.
    if isinstance(df.columns, pd.MultiIndex):
        df.columns = df.columns.get_level_values(0)

    validate_download(ticker, df)

    # Remove timezone information if present.
    if getattr(df.index, "tz", None) is not None:
        df.index = df.index.tz_localize(None)

    # ---------------------------------------------------------
    # Raw/audit dataset
    # ---------------------------------------------------------

    raw = df[
        [
            "Open",
            "High",
            "Low",
            "Close",
            "Adj Close",
            "Volume",
        ]
    ].copy()

    raw.index.name = "Date"

    raw_path = output_dir / f"{ticker}_raw.csv"

    raw.to_csv(
        raw_path,
        float_format="%.10f",
    )

    # ---------------------------------------------------------
    # QuantLab research dataset
    #
    # We explicitly use Adjusted Close as the research price.
    # This incorporates Yahoo's corporate-action adjustment.
    # ---------------------------------------------------------

    canonical = pd.DataFrame(
        {
            "Date": df.index.strftime("%Y-%m-%d"),
            "Open": df["Open"],
            "High": df["High"],
            "Low": df["Low"],
            "Close": df["Adj Close"],
            "Volume": df["Volume"],
        }
    )

    canonical_path = output_dir / f"{ticker}.csv"

    canonical.to_csv(
        canonical_path,
        index=False,
        float_format="%.10f",
    )

    # ---------------------------------------------------------
    # Diagnostics
    # ---------------------------------------------------------

    date_differences = (
        df.index.to_series()
        .diff()
        .dt.days
        .dropna()
    )

    max_gap = (
        int(date_differences.max())
        if not date_differences.empty
        else 0
    )

    print(f"  Rows:       {len(df)}")
    print(
        f"  Start:      "
        f"{df.index.min().date()}"
    )
    print(
        f"  End:        "
        f"{df.index.max().date()}"
    )
    print(f"  Max gap:    {max_gap} calendar days")
    print(f"  Raw file:   {raw_path}")
    print(f"  QuantLab:   {canonical_path}")


def main() -> None:
    
    project_root = (
        Path(__file__)
        .resolve()
        .parent
        .parent
    )

    output_dir = project_root / "data"

    output_dir.mkdir(
        parents=True,
        exist_ok=True,
    )

    print(
        "QuantLab historical data acquisition"
    )

    print(
        f"Start date: {START_DATE}"
    )

    print(
        "End date:   current available data"
    )

    print(
        f"Downloaded: "
        f"{datetime.now(timezone.utc).date()}"
    )

    for ticker in TICKERS:
        download_ticker(
            ticker=ticker,
            start_date=START_DATE,
            output_dir=output_dir,
        )

    print(
        "\nData acquisition completed successfully."
    )


if __name__ == "__main__":
    main()