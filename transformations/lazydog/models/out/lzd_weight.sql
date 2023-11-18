WITH healthkit_weight1 AS (
    SELECT
        substr(startDate, 1, 19) || substr(startDate, 21, 3) || ':' || substr(startDate, 24, 2) as dt,
        CAST(`value` as REAL) as weight_kg,
        'healthkit' as src
    FROM {{ ref('healthkit_bodymass') }}
    ORDER BY dt
),
gfit_weight2 AS (
    SELECT
        -- 10:00 o clock UTC is more assumable than 00:00
        (day_date || ' 10:00:00Z') as dt,
        average_kilo as weight_kg,
        'gfit' as src
    FROM {{ ref('gfit_weight_summary') }}
    ORDER BY dt
),
openscale_weight3 AS (
    SELECT
        datetime as dt,
        value as weight_kg,
        'openscale' as src
    FROM {{ ref('openscale_weight') }}
    WHERE type = 'weight'
    ORDER BY datetime
)

SELECT dt, weight_kg, src
FROM healthkit_weight1
UNION ALL
SELECT dt, weight_kg, src
FROM gfit_weight2
UNION ALL
SELECT dt, weight_kg, src
FROM openscale_weight3
ORDER BY dt, src
