
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
    CoinsPair NVARCHAR(7) NOT NULL,
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