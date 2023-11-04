SELECT
    *
FROM {{ source('healthkit', 'rSleepAnalysis') }}