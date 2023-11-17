SELECT
    *
FROM {{ source('openscale', 'openscale_measurements') }}
