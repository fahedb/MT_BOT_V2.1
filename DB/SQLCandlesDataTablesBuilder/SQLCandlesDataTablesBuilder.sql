/*
    Creates the core timeline data tables.

    Purpose:
    Stores current and historical OHLC candle data, plus multi-timeframe
    indicator values and slopes used by the research and C++ testing pipelines.

    Tables:
    - dbo.CurrentTimeLine
    - dbo.CurrentIndicatorTimeLine
    - dbo.HistoryTimeLine
    - dbo.HistoryIndicatorTimeLine

    Notes:
    - The source database uses HEAP tables with NONCLUSTERED UNIQUE constraints.
    - No foreign keys were defined in the source database export.
    - No production data is included.
*/

IF OBJECT_ID(N'dbo.CurrentTimeLine', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.CurrentTimeLine
    (
        TimePoint BIGINT NOT NULL,
        CoinsPair NVARCHAR(8) NOT NULL,

        OpenPrice DECIMAL(18, 8) NULL,
        ClosePrice DECIMAL(18, 8) NULL,
        HighPrice DECIMAL(18, 8) NULL,
        LowPrice DECIMAL(18, 8) NULL,

        CONSTRAINT UQ_CurrentTimeLine
            UNIQUE NONCLUSTERED (TimePoint ASC, CoinsPair ASC)
    );
END;
GO


IF OBJECT_ID(N'dbo.CurrentIndicatorTimeLine', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.CurrentIndicatorTimeLine
    (
        TimePoint BIGINT NOT NULL,
        CoinsPair NVARCHAR(8) NOT NULL,
        IndicatorName NVARCHAR(10) NOT NULL,
        TimeFrame NVARCHAR(4) NOT NULL,
        IndLen INT NOT NULL,

        IndicatorValue DECIMAL(18, 8) NULL,
        IndicatorSlope DECIMAL(18, 8) NULL,

        CONSTRAINT UQ_CurrentIndicatorTimeLine
            UNIQUE NONCLUSTERED
            (
                TimePoint ASC,
                CoinsPair ASC,
                IndicatorName ASC,
                TimeFrame ASC,
                IndLen ASC
            )
    );
END;
GO


IF OBJECT_ID(N'dbo.HistoryTimeLine', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.HistoryTimeLine
    (
        TimePoint BIGINT NOT NULL,
        CoinsPair NVARCHAR(8) NOT NULL,

        OpenPrice DECIMAL(18, 8) NULL,
        ClosePrice DECIMAL(18, 8) NULL,
        HighPrice DECIMAL(18, 8) NULL,
        LowPrice DECIMAL(18, 8) NULL,

        CONSTRAINT UQ_HistoryTimeLine
            UNIQUE NONCLUSTERED (TimePoint ASC, CoinsPair ASC)
    );
END;
GO


IF OBJECT_ID(N'dbo.HistoryIndicatorTimeLine', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.HistoryIndicatorTimeLine
    (
        TimePoint BIGINT NOT NULL,
        CoinsPair NVARCHAR(8) NOT NULL,
        IndicatorName NVARCHAR(10) NOT NULL,
        TimeFrame NVARCHAR(4) NOT NULL,
        IndLen INT NOT NULL,

        IndicatorValue DECIMAL(18, 8) NULL,
        IndicatorSlope DECIMAL(18, 8) NULL,

        CONSTRAINT UQ_HistoryIndicatorTimeLine
            UNIQUE NONCLUSTERED
            (
                TimePoint ASC,
                CoinsPair ASC,
                IndicatorName ASC,
                TimeFrame ASC,
                IndLen ASC
            )
    );
END;
GO
