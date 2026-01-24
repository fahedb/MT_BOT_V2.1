
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

    CONSTRAINT PK_BackTest_AccountStateEvents PRIMARY KEY (SimulationID, ticket)
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
