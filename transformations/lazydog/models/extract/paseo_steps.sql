SELECT 
    *
FROM {{ source('paseo', 'hours') }}