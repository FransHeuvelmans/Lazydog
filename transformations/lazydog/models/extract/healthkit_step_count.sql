SELECT
    *
FROM {{ source('healthkit', 'rStepCount') }}