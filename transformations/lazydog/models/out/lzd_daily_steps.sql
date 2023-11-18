WITH gfit_daily_steps1 AS (
    SELECT
        day_date,
        step_count,
        'gfit' as src
    FROM {{ ref('gfit_daily_fit_summary') }}
    WHERE step_count > 0
    ORDER BY day_date
),
healthkit_daily_steps2 AS (
    SELECT
        day_date,
        SUM(step_count_part) as step_count,
        'healthkit' as src
    FROM
        (SELECT
            -- Substring to get the date string out of the datetime
            substr(startDate, 1, 10) as day_date,
            CAST(`value` as INTEGER) as step_count_part
        FROM {{ ref('healthkit_step_count') }}
        WHERE step_count_part > 0)
    GROUP BY day_date
    ORDER BY day_date
),
paseo_daily_steps3 AS (
    SELECT
        day_date,
        SUM(step_count_part) as step_count,
        'paseo' as src
    FROM
        (SELECT
            -- Substring to get the date string out of the datetime
            substr(start_hour, 1, 10) as day_date,
            CAST(`steps_in_hour` as INTEGER) as step_count_part
        FROM {{ ref('paseo_clean_steps') }}
        WHERE step_count_part > 0)
    GROUP BY day_date
    ORDER BY day_date
)

SELECT day_date, step_count, src
FROM gfit_daily_steps1
UNION ALL
SELECT day_date, step_count, src
FROM healthkit_daily_steps2
UNION ALL
SELECT day_date, step_count, src
FROM paseo_daily_steps3
ORDER BY day_date, src
