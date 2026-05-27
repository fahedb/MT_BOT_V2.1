
CREATE TABLE dbo.BackTest_Deals (
    SimulationID      INT            NOT NULL, 
    symbol            NVARCHAR(32)    NOT NULL,
    ticket            BIGINT         NOT NULL, 
    openTime          BIGINT         NOT NULL, 
    closeTime         BIGINT         NOT NULL, 
    lotSize           DECIMAL(18, 8)  NOT NULL,
    openPrice         DECIMAL(18, 8)  NOT NULL,
    closePrice        DECIMAL(18, 8)  NOT NULL,
    takeProfitPrice   DECIMAL(18, 8)  NOT NULL,
    stopLossPrice     DECIMAL(18, 8)  NOT NULL,
    dealType          NVARCHAR(16)    NOT NULL,
    profit            DECIMAL(18, 8)  NOT NULL,
    profitInPips      INT            NOT NULL, 
    closeReason       NVARCHAR(32)    NOT NULL,
    numOfBarsHeld     INT              NOT NULL,

    CONSTRAINT PK_BackTest_Deals PRIMARY KEY (SimulationID, ticket)
);

CREATE TABLE dbo.BackTest_AccountStateEvents (
    SimulationID     INT            NOT NULL,
    CurrTimePoint    BIGINT         NOT NULL, -- YYYYMMDDHHMMSSMMM
    AccountBalance   DECIMAL(18, 8) NOT NULL,
    EquityBalance    DECIMAL(18, 8) NOT NULL,
    UsedMargin       DECIMAL(18, 8) NOT NULL,
    FreeMargin       DECIMAL(18, 8) NOT NULL,
    FloatingPnL      DECIMAL(18, 8) NOT NULL,
    CurrProfit       DECIMAL(18, 8) NOT NULL,

    CONSTRAINT PK_BackTest_AccountStateEvents
        PRIMARY KEY (SimulationID, CurrTimePoint)
);

CREATE TABLE dbo.BackTest_DirectionStats (
    SimulationID int not null,
    TimePoint BIGINT NOT NULL,
    CoinsPair NVARCHAR(8) NOT NULL,
    VoteType NVARCHAR(10) not null,
    TotalVotes int not null,
    UpPercentage DECIMAL(18, 8) NOT NULL,
    DownPercentage DECIMAL(18, 8) NOT NULL,
    BlurPercentage DECIMAL(18, 8) NOT NULL,
    Mean DECIMAL(18, 8) NOT NULL,
    PureMean_Up DECIMAL(18, 8) NOT NULL,
    PureMean_Down DECIMAL(18, 8) NOT NULL,
    StdDev DECIMAL(18, 8) NOT NULL,
    MeanIndexes DECIMAL(18, 8) NOT NULL,
    TradeResult int not null

        CONSTRAINT PK_BackTest_DirectionStats
        PRIMARY KEY (SimulationID, VoteType, TimePoint)
);


CREATE TABLE dbo.HistoryTimeLine
(
    TimePoint BIGINT NOT NULL,
    CoinsPair NVARCHAR(8) NOT NULL,
    OpenPrice DECIMAL(18,8),
    ClosePrice DECIMAL(18,8),
    HighPrice DECIMAL(18,8),
    LowPrice DECIMAL(18,8),
    CONSTRAINT UQ_HistoryTimeLine UNIQUE (TimePoint, CoinsPair)
);

CREATE TABLE dbo.HistoryIndicatorTimeLine
(
    TimePoint BIGINT NOT NULL,
    CoinsPair NVARCHAR(8) NOT NULL,
    IndicatorName NVARCHAR(10) NOT NULL,
    TimeFrame NVARCHAR(4) NOT NULL,
    IndLen int not null,
    IndicatorValue DECIMAL(18,8),
    IndicatorSlope DECIMAL(18,8),
    CONSTRAINT UQ_HistoryIndicatorTimeLine 
    UNIQUE (TimePoint, CoinsPair, IndicatorName, TimeFrame, IndLen)
);

CREATE TABLE dbo.CurrentTimeLine
(
    TimePoint BIGINT NOT NULL,
    CoinsPair NVARCHAR(8) NOT NULL,
    OpenPrice DECIMAL(18,8),
    ClosePrice DECIMAL(18,8),
    HighPrice DECIMAL(18,8),
    LowPrice DECIMAL(18,8),
    CONSTRAINT UQ_CurrentTimeLine UNIQUE (TimePoint, CoinsPair)
);

CREATE TABLE dbo.CurrentIndicatorTimeLine
(
    TimePoint BIGINT NOT NULL,
    CoinsPair NVARCHAR(8) NOT NULL,
    IndicatorName NVARCHAR(10) NOT NULL,
    TimeFrame NVARCHAR(4) NOT NULL,
    IndLen int not null,
    IndicatorValue DECIMAL(18,8),
    IndicatorSlope DECIMAL(18,8),
    CONSTRAINT UQ_CurrentIndicatorTimeLine
    UNIQUE (TimePoint, CoinsPair, IndicatorName, TimeFrame, IndLen)
);

CREATE TABLE dbo.FutureStats (
    SimulationID INT NOT NULL,
    TimePoint BIGINT NOT NULL,
    CoinsPair NVARCHAR(20) NOT NULL,

    TP_HitIndex INT NULL,
    SL_HitIndex INT NULL,
    IsWin BIT NULL,
    FirstHitType INT NULL
);

CREATE TABLE dbo.ML_ExperimentResults (
    ID INT IDENTITY(1,1) PRIMARY KEY,

    SimulationID INT,
    ModelType NVARCHAR(50),

    MaxDepth INT NULL,

    Accuracy FLOAT,

    Threshold FLOAT,
    TotalTrades INT,
    WinRate FLOAT,

    CreatedAt DATETIME DEFAULT GETDATE()
);

use Prices
CREATE TABLE dbo.ML_RuleResults
(
    ID INT IDENTITY(1,1) PRIMARY KEY,

    RuleDescription NVARCHAR(200),
    StatType NVARCHAR(20),

    -- TRAIN
    TrainTrades INT,
    TrainWinRate FLOAT,
    TrainEdge FLOAT,

    -- TEST
    TestTrades INT,
    TestWinRate FLOAT,
    TestEdge FLOAT,

    -- ANALYSIS
    Delta FLOAT,
    Stability FLOAT,
    Score FLOAT,

    CreatedAt DATETIME DEFAULT GETDATE()
);