SELECT
	_id,
	date,
	count(1) as cnt
FROM {{ ref('paseo_steps') }}
GROUP BY _id, date
HAVING cnt > 1
