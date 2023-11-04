SELECT 
	_id as source_id,
	datetime(substr(`date`, 1, 4) || '-' || substr(`date`, 5, 2) || '-' || substr(`date`, 7, 2) || 'T' || PRINTF('%02d',`hour`) || ':00') as `start_hour`,
	(endSteps - startSteps) as steps_in_hour
FROM {{ ref('paseo_steps') }};