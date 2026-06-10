/*
    Creates the VectorMatchRuns table.

    Purpose:
    Stores metadata for each vector-matching research run, including the
    extractor configuration, matching configuration, vector dimensions,
    threshold settings, output file path, and creation timestamp.

    Notes:
    - This script intentionally uses readable constraint names instead of
      SQL Server auto-generated names.
    - No production data is included.
*/

IF OBJECT_ID(N'dbo.VectorMatchRuns', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.VectorMatchRuns
    (
        MatchRunID INT IDENTITY(1,1) NOT NULL,

        ExtractorsJson NVARCHAR(MAX) NOT NULL,
        ConfigJson NVARCHAR(MAX) NULL,

        VectorSize INT NULL,
        WindowSize INT NULL,
        TopK INT NULL,
        DistanceThreshold FLOAT NULL,

        NormalizationMethod NVARCHAR(100) NULL,
        MatchesFile NVARCHAR(500) NULL,

        CreatedAt DATETIME2(7) NOT NULL
            CONSTRAINT DF_VectorMatchRuns_CreatedAt
            DEFAULT SYSUTCDATETIME(),

        CONSTRAINT PK_VectorMatchRuns
            PRIMARY KEY CLUSTERED (MatchRunID)
    );
END;
GO


--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------


/*
    Creates the VectorEdgeRuns table.

    Purpose:
    Stores evaluation metrics for a vector-matching research run.
    Each row is linked to a matching run in dbo.VectorMatchRuns and contains
    direction accuracy, return correlation, top-percentile performance metrics,
    thresholds, and optional notes.

    Notes:
    - This script uses readable constraint names instead of SQL Server
      auto-generated names.
    - No production data is included.
*/

IF OBJECT_ID(N'dbo.VectorEdgeRuns', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.VectorEdgeRuns
    (
        EdgeRunID INT IDENTITY(1,1) NOT NULL,
        MatchRunID INT NOT NULL,

        EvaluationConfigJson NVARCHAR(MAX) NULL,
        DatasetRows INT NULL,

        PredReturnCorr FLOAT NULL,
        DirectionAccuracy FLOAT NULL,
        BaselineReturn FLOAT NULL,

        Top50Accuracy FLOAT NULL,
        Top50Samples INT NULL,
        Top50AvgReturn FLOAT NULL,

        Top40Accuracy FLOAT NULL,
        Top40Samples INT NULL,
        Top40AvgReturn FLOAT NULL,

        Top30Accuracy FLOAT NULL,
        Top30Samples INT NULL,
        Top30AvgReturn FLOAT NULL,

        Top20Accuracy FLOAT NULL,
        Top20Samples INT NULL,
        Top20AvgReturn FLOAT NULL,

        Top10Accuracy FLOAT NULL,
        Top10Samples INT NULL,
        Top10AvgReturn FLOAT NULL,

        Top5Accuracy FLOAT NULL,
        Top5Samples INT NULL,
        Top5AvgReturn FLOAT NULL,

        Notes NVARCHAR(MAX) NULL,

        CreatedAt DATETIME2(7) NOT NULL
            CONSTRAINT DF_VectorEdgeRuns_CreatedAt
            DEFAULT SYSUTCDATETIME(),

        AvgHistReturnCorr FLOAT NULL,
        MedianHistReturnCorr FLOAT NULL,

        Top50Threshold FLOAT NULL,
        Top40Threshold FLOAT NULL,
        Top30Threshold FLOAT NULL,
        Top20Threshold FLOAT NULL,
        Top10Threshold FLOAT NULL,
        Top5Threshold FLOAT NULL,

        CONSTRAINT PK_VectorEdgeRuns
            PRIMARY KEY CLUSTERED (EdgeRunID),

        CONSTRAINT FK_VectorEdgeRuns_VectorMatchRuns
            FOREIGN KEY (MatchRunID)
            REFERENCES dbo.VectorMatchRuns (MatchRunID)
    );
END;
GO


--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------


/*
    Creates the StrategyTestRuns table.

    Purpose:
    Stores strategy simulation / backtest run metadata, signal filters,
    edge-related parameters, simulator settings, execution statistics,
    performance metrics, output artifact paths, and creation timestamp.

    Notes:
    - This script reflects the exported database structure.
    - EdgeRunID and MatchRunID are indexed but no foreign keys were defined
      in the source database export.
    - No production data is included.
*/

IF OBJECT_ID(N'dbo.StrategyTestRuns', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.StrategyTestRuns
    (
        StrategyRunID BIGINT IDENTITY(1,1) NOT NULL,

        EdgeRunID INT NULL,
        MatchRunID INT NULL,

        EdgeLevel NVARCHAR(20) NULL,
        EdgeThreshold FLOAT NULL,
        EdgeConfigName NVARCHAR(255) NULL,
        MatchesFile NVARCHAR(500) NULL,

        SignalDirectionMethod NVARCHAR(100) NULL,
        FutureDirectionMethod NVARCHAR(100) NULL,

        FirstHitPips INT NULL,
        FirstHitFutureWindow INT NULL,

        MinAbsPredReturn FLOAT NULL,
        MinDirectionAgreement FLOAT NULL,
        MinWeightedDirectionAgreement FLOAT NULL,
        MinFirstHitAgreement FLOAT NULL,
        MinWeightedFirstHitAgreement FLOAT NULL,
        MinValidFirstHitCount INT NULL,
        MaxFirstHitNoiseRatio FLOAT NULL,

        EdgeDatasetRows INT NULL,
        EdgeDirectionAccuracy FLOAT NULL,
        EdgeLevelAccuracy FLOAT NULL,
        EdgeLevelSamples INT NULL,
        EdgeLevelAvgReturn FLOAT NULL,

        Symbol NVARCHAR(50) NULL,
        PipSize FLOAT NULL,
        MoneyPerPip FLOAT NULL,
        SpreadPips FLOAT NULL,
        SameCandlePolicy NVARCHAR(50) NULL,

        TpPips FLOAT NULL,
        SlPips FLOAT NULL,
        LotSize FLOAT NULL,
        MaxOpenTrades INT NULL,
        BlockReentrySameCandle BIT NULL,
        MaxCandles INT NULL,

        CurrentBarsProcessed INT NULL,
        SignalsChecked INT NULL,
        SignalsPassed INT NULL,
        TradesOpened INT NULL,
        TotalTrades INT NULL,
        WinningTrades INT NULL,
        LosingTrades INT NULL,
        ClosedTrades INT NULL,
        OpenTradesAtEnd INT NULL,

        WinRate FLOAT NULL,
        LossRate FLOAT NULL,

        InitialBalance FLOAT NULL,
        FinalBalance FLOAT NULL,
        NetProfit FLOAT NULL,
        NetProfitPct FLOAT NULL,
        GrossProfit FLOAT NULL,
        GrossLoss FLOAT NULL,
        ProfitFactor FLOAT NULL,
        MaxDrawdown FLOAT NULL,
        MaxDrawdownPct FLOAT NULL,

        AvgProfitPerTrade FLOAT NULL,
        AvgWin FLOAT NULL,
        AvgLoss FLOAT NULL,
        BestTrade FLOAT NULL,
        WorstTrade FLOAT NULL,

        TpCount INT NULL,
        SlCount INT NULL,
        SameCandleTpCount INT NULL,
        SameCandleSlCount INT NULL,
        EndOfTestCount INT NULL,

        BuyTrades INT NULL,
        SellTrades INT NULL,
        BuyWins INT NULL,
        SellWins INT NULL,
        BuyWinRate FLOAT NULL,
        SellWinRate FLOAT NULL,

        SimulatorConfigJson NVARCHAR(MAX) NULL,
        SignalConfigJson NVARCHAR(MAX) NULL,
        StrategyConfigJson NVARCHAR(MAX) NULL,
        RunMetadataJson NVARCHAR(MAX) NULL,
        SummaryJson NVARCHAR(MAX) NULL,

        TradesCsvPath NVARCHAR(1000) NULL,
        TradesJsonPath NVARCHAR(1000) NULL,
        SummaryJsonPath NVARCHAR(1000) NULL,

        CreatedAt DATETIME2(7) NOT NULL
            CONSTRAINT DF_StrategyTestRuns_CreatedAt
            DEFAULT SYSUTCDATETIME(),

        CONSTRAINT PK_StrategyTestRuns
            PRIMARY KEY CLUSTERED (StrategyRunID)
    );
END;
GO

IF NOT EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_StrategyTestRuns_CreatedAt'
      AND object_id = OBJECT_ID(N'dbo.StrategyTestRuns')
)
BEGIN
    CREATE NONCLUSTERED INDEX IX_StrategyTestRuns_CreatedAt
    ON dbo.StrategyTestRuns (CreatedAt DESC);
END;
GO

IF NOT EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_StrategyTestRuns_EdgeRunID'
      AND object_id = OBJECT_ID(N'dbo.StrategyTestRuns')
)
BEGIN
    CREATE NONCLUSTERED INDEX IX_StrategyTestRuns_EdgeRunID
    ON dbo.StrategyTestRuns (EdgeRunID ASC);
END;
GO

IF NOT EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_StrategyTestRuns_MatchRunID'
      AND object_id = OBJECT_ID(N'dbo.StrategyTestRuns')
)
BEGIN
    CREATE NONCLUSTERED INDEX IX_StrategyTestRuns_MatchRunID
    ON dbo.StrategyTestRuns (MatchRunID ASC);
END;
GO

IF NOT EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_StrategyTestRuns_NetProfit'
      AND object_id = OBJECT_ID(N'dbo.StrategyTestRuns')
)
BEGIN
    CREATE NONCLUSTERED INDEX IX_StrategyTestRuns_NetProfit
    ON dbo.StrategyTestRuns (NetProfit DESC);
END;
GO

IF NOT EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_StrategyTestRuns_ProfitFactor'
      AND object_id = OBJECT_ID(N'dbo.StrategyTestRuns')
)
BEGIN
    CREATE NONCLUSTERED INDEX IX_StrategyTestRuns_ProfitFactor
    ON dbo.StrategyTestRuns (ProfitFactor DESC);
END;
GO

IF NOT EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_StrategyTestRuns_WinRate'
      AND object_id = OBJECT_ID(N'dbo.StrategyTestRuns')
)
BEGIN
    CREATE NONCLUSTERED INDEX IX_StrategyTestRuns_WinRate
    ON dbo.StrategyTestRuns (WinRate DESC);
END;
GO
