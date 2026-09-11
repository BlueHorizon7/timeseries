import yfinance as yf
import pandas as pd
import numpy as np

def fetch_and_validate_data(tickers: list, start_date: str, end_date: str) -> tuple:
    """
    Downloads historical data and generates a strict data quality report.
    Fails loudly if the dataset is insufficient for quant research.
    """
    print(f"Fetching data for {tickers} from {start_date} to {end_date}...")

    data = yf.download(tickers, start=start_date, end=end_date, auto_adjust=False)

    clean_data = {}
    dq_report = {}

    MIN_REQUIRED_DAYS = 1260

    for ticker in tickers:
        df = data.xs(ticker, level='Ticker', axis=1) if len(tickers) > 1 else data.copy()
        expected_days = pd.date_range(start=df.index.min(), end=df.index.max(), freq='B')
        missing_dates = expected_days.difference(df.index)
        duplicates = df.index.duplicated().sum()
        non_finite = df.isna().sum().to_dict()

        ohlc_inconsistent = (
            (df['High'] < df['Open']) |
            (df['High'] < df['Close']) |
            (df['Low'] > df['Open']) |
            (df['Low'] > df['Close'])
        ).sum()

        dq_report[ticker] = {
            'total_rows': len(df),
            'missing_business_days_count': len(missing_dates),
            'duplicate_rows': duplicates,
            'missing_adj_close': non_finite.get('Adj Close', 0),
            'ohlc_inconsistencies': ohlc_inconsistent
        }

        if len(df) < MIN_REQUIRED_DAYS:
            raise ValueError(f"DATA REJECTED: {ticker} has {len(df)} days. Minimum required is {MIN_REQUIRED_DAYS}.")
        
        if duplicates > 0:
            raise ValueError(f"DATA REJECTED: {ticker} contains {duplicates} duplicate timestamps.")
            
        if non_finite.get('Adj Close', 0) > 0:
            # Forward fill small gaps, but reject if there are too many
            if non_finite['Adj Close'] > 5:
                raise ValueError(f"DATA REJECTED: {ticker} has missing Adjusted Close prices.")
            else:
                df.ffill(inplace=True)
                
        clean_data[ticker] = df

    return clean_data, dq_report

if __name__ == "__main__":
    try:
        # Fetch 10 years of data
        datasets, report = fetch_and_validate_data(
            tickers=["KO", "PEP"], 
            start_date="2014-01-01", 
            end_date="2024-01-01"
        )
        
        print("\n=== Data Quality Report ===")
        for tkr, metrics in report.items():
            print(f"\n{tkr}:")
            for k, v in metrics.items():
                print(f"  - {k}: {v}")
                
        print("\nStage 1 Complete: Data is clean and ready for statistical research.")
        
    except ValueError as e:
        print(f"\nPIPELINE HALTED: {e}")