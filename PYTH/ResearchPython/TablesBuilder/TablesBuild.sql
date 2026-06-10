CREATE TABLE dbo.StrategyTestRuns
(
    StrategyRunID BIGINT IDENTITY(1,1) NOT NULL PRIMARY KEY,

    -- =========================
    -- Link to Edge / Match
    -- =========================
    EdgeRunID INT NULL,
    MatchRunID INT NULL,
    EdgeLevel NVARCHAR(20) NULL,
    EdgeThreshold FLOAT NULL,
    EdgeConfigName NVARCHAR(255) NULL,

    MatchesFile NVARCHAR(500) NULL,

    -- =========================
    -- Strategy / Signal metadata
    -- =========================
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

    -- =========================
    -- Simulator config
    -- =========================
    Symbol NVARCHAR(50) NULL,
    PipSize FLOAT NULL,
    MoneyPerPip FLOAT NULL,
    SpreadPips FLOAT NULL,
    SameCandlePolicy NVARCHAR(50) NULL,

    -- =========================
    -- Strategy config
    -- =========================
    TpPips FLOAT NULL,
    SlPips FLOAT NULL,
    LotSize FLOAT NULL,
    MaxOpenTrades INT NULL,
    BlockReentrySameCandle BIT NULL,
    MaxCandles INT NULL,

    -- =========================
    -- Test coverage
    -- =========================
    CurrentBarsProcessed INT NULL,
    SignalsChecked INT NULL,
    SignalsPassed INT NULL,
    TradesOpened INT NULL,

    -- =========================
    -- Trading results
    -- =========================
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

    -- =========================
    -- Extra analysis
    -- =========================
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

    -- =========================
    -- Raw configs / full data
    -- =========================
    SimulatorConfigJson NVARCHAR(MAX) NULL,
    SignalConfigJson NVARCHAR(MAX) NULL,
    StrategyConfigJson NVARCHAR(MAX) NULL,
    RunMetadataJson NVARCHAR(MAX) NULL,
    SummaryJson NVARCHAR(MAX) NULL,

    -- =========================
    -- Output files
    -- =========================
    TradesCsvPath NVARCHAR(1000) NULL,
    TradesJsonPath NVARCHAR(1000) NULL,
    SummaryJsonPath NVARCHAR(1000) NULL,

    -- =========================
    -- Time
    -- =========================
    CreatedAt DATETIME2 NOT NULL DEFAULT SYSUTCDATETIME()
);


CREATE INDEX IX_StrategyTestRuns_EdgeRunID
ON dbo.StrategyTestRuns (EdgeRunID);

CREATE INDEX IX_StrategyTestRuns_MatchRunID
ON dbo.StrategyTestRuns (MatchRunID);

CREATE INDEX IX_StrategyTestRuns_CreatedAt
ON dbo.StrategyTestRuns (CreatedAt DESC);

CREATE INDEX IX_StrategyTestRuns_NetProfit
ON dbo.StrategyTestRuns (NetProfit DESC);

CREATE INDEX IX_StrategyTestRuns_WinRate
ON dbo.StrategyTestRuns (WinRate DESC);

CREATE INDEX IX_StrategyTestRuns_ProfitFactor
ON dbo.StrategyTestRuns (ProfitFactor DESC);