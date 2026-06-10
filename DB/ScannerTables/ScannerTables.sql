/*
    Creates the scanner infrastructure tables.

    Purpose:
    Stores runtime scanner configuration values and generic sequence counters
    used by the scanner/backend components.

    Tables:
    - dbo.configs
    - dbo.seq_nums

    Notes:
    - Constraint names were normalized for readability.
    - The source database uses dbo.seq_nums as a HEAP table with a UNIQUE
      NONCLUSTERED constraint on entity.
    - No production data is included.
*/

IF OBJECT_ID(N'dbo.configs', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.configs
    (
        id INT IDENTITY(1,1) NOT NULL,

        config_name NVARCHAR(100) NOT NULL,
        config_value NVARCHAR(20) NOT NULL,
        config_explain NVARCHAR(4000) NULL,
        config_permission INT NULL,
        config_catagory INT NULL,

        CONSTRAINT PK_configs
            PRIMARY KEY CLUSTERED (id ASC),

        CONSTRAINT UQ_configs_config_name
            UNIQUE NONCLUSTERED (config_name ASC)
    );
END;
GO


IF OBJECT_ID(N'dbo.seq_nums', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.seq_nums
    (
        entity VARCHAR(255) NULL,
        seq BIGINT NULL,

        CONSTRAINT UQ_seq_nums_entity
            UNIQUE NONCLUSTERED (entity ASC)
    );
END;
GO
