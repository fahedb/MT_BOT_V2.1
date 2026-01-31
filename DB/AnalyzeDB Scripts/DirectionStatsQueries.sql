

-- Total profit summery
SELECT
    COUNT(*)                AS Trades,
    SUM(profit)             AS TotalProfit,
    AVG(profit)             AS AvgProfitPerTrade,
    MIN(profit)             AS MinProfit,
    MAX(profit)             AS MaxProfit
FROM dbo.BackTest_Deals;

-------------------------------------------------------------------
-- PureMean sweep table for LONG

WITH Thresholds AS (
    SELECT v.threshold
    FROM (VALUES
        (100),(110),(120),(130),(140),(150),
        (160),(170),(180),(190),(200),(210),(220)
    ) v(threshold)
),
Filtered AS (
    SELECT
        t.threshold,
        d.CloseReason,
        d.profit
    FROM dbo.BackTest_Deals d
    JOIN dbo.BackTest_DirectionStats ds
        ON ds.TimePoint = d.OpenTime
       AND ds.VoteType = 'General'
    JOIN Thresholds t
        ON ds.PureMean_Up >= t.threshold
    WHERE d.dealType = 'LONG'
)
SELECT
    threshold,
    COUNT(*) AS Trades,
    SUM(CASE WHEN CloseReason = 'TAKE_PROFIT_HIT' THEN 1 ELSE 0 END) AS Wins,
    CAST(
        SUM(CASE WHEN CloseReason = 'TAKE_PROFIT_HIT' THEN 1 ELSE 0 END) * 100.0
        / NULLIF(COUNT(*), 0)
        AS DECIMAL(5,2)
    ) AS WinRate,

    SUM(profit) AS TotalProfit,
    AVG(profit) AS AvgProfitPerTrade,

    AVG(CASE WHEN profit > 0 THEN profit END) AS AvgWinProfit,
    AVG(CASE WHEN profit < 0 THEN profit END) AS AvgLossProfit
FROM Filtered
GROUP BY threshold
ORDER BY threshold;

------------------------------------------------------------------------------------------------------
-- PureMean sweep table for SHORT

WITH Thresholds AS (
    SELECT v.threshold
    FROM (VALUES
        (100),(110),(120),(130),(140),(150),
        (160),(170),(180),(190),(200),(210),(220)
    ) v(threshold)
),
Filtered AS (
    SELECT
        t.threshold,
        d.CloseReason,
        d.profit
    FROM dbo.BackTest_Deals d
    JOIN dbo.BackTest_DirectionStats ds
        ON ds.TimePoint = d.OpenTime
       AND ds.VoteType = 'General'
    JOIN Thresholds t
        ON ds.PureMean_Down <= -t.threshold   
    WHERE d.dealType = 'SHORT'
)
SELECT
    threshold,
    COUNT(*) AS Trades,
    SUM(CASE WHEN CloseReason = 'TAKE_PROFIT_HIT' THEN 1 ELSE 0 END) AS Wins,
    CAST(
        SUM(CASE WHEN CloseReason = 'TAKE_PROFIT_HIT' THEN 1 ELSE 0 END) * 100.0
        / NULLIF(COUNT(*), 0)
        AS DECIMAL(5,2)
    ) AS WinRate,

    SUM(profit) AS TotalProfit,
    AVG(profit) AS AvgProfitPerTrade,

    AVG(CASE WHEN profit > 0 THEN profit END) AS AvgWinProfit,
    AVG(CASE WHEN profit < 0 THEN profit END) AS AvgLossProfit
FROM Filtered
GROUP BY threshold
ORDER BY threshold;

------------------------------------------------------------------------------------------
-- Total votes sweep table

WITH VoteThresholds AS (
    SELECT v.votes
    FROM (VALUES
        (3),(4),(5),(6),(7),(8),(9),(10),
        (11),(12),(13),(14),(15),(16),(17),(18),(19),(20)
    ) v(votes)
),
Filtered AS (
    SELECT
        vt.votes        AS VotesThreshold,
        d.dealType,
        d.CloseReason,
        d.profit
    FROM dbo.BackTest_Deals d
    JOIN dbo.BackTest_DirectionStats ds
        ON ds.TimePoint = d.OpenTime
       AND ds.VoteType = 'General'
    JOIN VoteThresholds vt
        ON ds.TotalVotes >= vt.votes
    WHERE
        (
            d.dealType = 'LONG'
            AND ds.PureMean_Up >= 160
        )
        OR
        (
            d.dealType = 'SHORT'
            AND ds.PureMean_Down <= -140
        )
)
SELECT
    VotesThreshold,
    COUNT(*)                         AS Trades,
    SUM(CASE WHEN CloseReason = 'TAKE_PROFIT_HIT' THEN 1 ELSE 0 END) AS Wins,
    CAST(
        SUM(CASE WHEN CloseReason = 'TAKE_PROFIT_HIT' THEN 1 ELSE 0 END) * 100.0
        / NULLIF(COUNT(*), 0)
        AS DECIMAL(5,2)
    ) AS WinRate,

    SUM(profit) AS TotalProfit,
    AVG(profit) AS AvgProfitPerTrade
FROM Filtered
GROUP BY VotesThreshold
ORDER BY VotesThreshold;
