SELECT
    *
FROM {{ source('healthkit', 'rFlightsClimbed') }}