/*
    Creates scanner research/result tables.

    Purpose:
    Stores research outputs generated from C++ scanner/backtest runs, including
    future TP/SL hit statistics, ML experiment summaries, rule-based experiment
    summaries, and temporary hit-analysis data.

    Tables:
    - dbo.FutureStats
    - dbo.ML_ExperimentResults
    - dbo.ML_RuleResults
    - dbo.TempHits

    Notes:
    - This script reflects the exported database structure.
    - Most tables are research/result tables and do not include foreign keys.
    - No production data is included.
*/

IF OBJECT_ID(N'dbo.FutureStats', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.FutureStats
    (
        SimulationID INT NOT NULL,
        TimePoint BIGINT NOT NULL,
        CoinsPair NVARCHAR(20) NOT NULL,

        TP_HitIndex INT NULL,
        SL_HitIndex INT NULL,
        IsWin BIT NULL,
        FirstHitType INT NULL
    );
END;
GO

IF NOT EXISTS (
    SELECT 1
    FROM sys.indexes
    WHERE name = N'IX_FutureStats_Sim_Time'
      AND object_id = OBJECT_ID(N'dbo.FutureStats')
)
BEGIN
    CREATE NONCLUSTERED INDEX IX_FutureStats_Sim_Time
    ON dbo.FutureStats (SimulationID ASC, TimePoint ASC);
END;
GO


IF OBJECT_ID(N'dbo.ML_ExperimentResults', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.ML_ExperimentResults
    (
        ID INT IDENTITY(1,1) NOT NULL,

        SimulationID INT NULL,
        ModelType NVARCHAR(50) NULL,
        MaxDepth INT NULL,

        Accuracy FLOAT NULL,
        Threshold FLOAT NULL,
        TotalTrades INT NULL,
        WinRate FLOAT NULL,

        CreatedAt DATETIME NULL
            CONSTRAINT DF_ML_ExperimentResults_CreatedAt
            DEFAULT GETDATE(),

        Edge FLOAT NULL,
        StatType NVARCHAR(20) NULL,
        Score FLOAT NULL,

        CONSTRAINT PK_ML_ExperimentResults
            PRIMARY KEY CLUSTERED (ID ASC)
    );
END;
GO


IF OBJECT_ID(N'dbo.ML_RuleResults', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.ML_RuleResults
    (
        ID INT IDENTITY(1,1) NOT NULL,

        RuleDescription NVARCHAR(200) NULL,
        StatType NVARCHAR(20) NULL,

        TrainTrades INT NULL,
        TrainWinRate FLOAT NULL,
        TrainEdge FLOAT NULL,

        TestTrades INT NULL,
        TestWinRate FLOAT NULL,
        TestEdge FLOAT NULL,

        Delta FLOAT NULL,
        Stability FLOAT NULL,
        Score FLOAT NULL,

        CreatedAt DATETIME NULL
            CONSTRAINT DF_ML_RuleResults_CreatedAt
            DEFAULT GETDATE(),

        CONSTRAINT PK_ML_RuleResults
            PRIMARY KEY CLUSTERED (ID ASC)
    );
END;
GO


IF OBJECT_ID(N'dbo.TempHits', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.TempHits
    (
        TimePoint BIGINT NULL,
        ClosePrice FLOAT NULL,
        TP_HitIndex FLOAT NULL,
        SL_HitIndex FLOAT NULL,
        IsWin BIGINT NULL
    );
END;
GO
