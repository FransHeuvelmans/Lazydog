SELECT
    *
FROM {{ source('healthkit', 'rDistanceWalkingRunning') }}