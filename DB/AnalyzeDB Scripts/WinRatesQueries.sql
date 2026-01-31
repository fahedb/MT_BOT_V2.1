-- Counts 
select count(*) as ShortWin from BackTest_Deals where CloseReason = 'TAKE_PROFIT_HIT' and dealType = 'SHORT'
select count(*) as ShortLose from BackTest_Deals where CloseReason = 'STOP_LOSS_HIT' and dealType = 'SHORT'
select count(*) as LongWin from BackTest_Deals where CloseReason = 'TAKE_PROFIT_HIT' and dealType = 'LONG'
select count(*) as LongLose from BackTest_Deals where CloseReason = 'STOP_LOSS_HIT' and dealType = 'LONG'

-- Win rates 
SELECT
    dealType,
    COUNT(*) AS Total,
    SUM(CASE WHEN profit > 0 THEN 1 ELSE 0 END) AS Wins,
    SUM(CASE WHEN profit <= 0 THEN 1 ELSE 0 END) AS Losses,
    CAST(
        SUM(CASE WHEN profit > 0 THEN 1 ELSE 0 END) * 100.0
        / NULLIF(COUNT(*), 0)
        AS DECIMAL(5,2)
    ) AS WinRatePercent
FROM dbo.BackTest_Deals 
GROUP BY dealType;

-- Max DrawDown 

select min(AccountBalance) as Balace_DrawDown from BackTest_AccountStateEvents
select min(EquityBalance) as Equity_DrawDown from BackTest_AccountStateEvents

