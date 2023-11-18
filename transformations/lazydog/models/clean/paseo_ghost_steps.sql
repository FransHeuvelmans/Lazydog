with temp_hour_diffs AS (
SELECT 
	`_id`,
    -- Difference since last day
	(startSteps - LAG(endSteps, 1) OVER (ORDER BY `_id`)) AS step_difference
FROM {{ ref('paseo_steps') }}
)


-- Generally rows with 0 found steps and a large negative step_difference are resets
-- Small positive changes at the start of the day are probably recorded by Android at some point but not logged on the hour
SELECT 
	tab_hours._id as source_id,
	datetime(substr(tab_hours.date, 1, 4) || '-' || substr(tab_hours.date, 5, 2) || '-' || substr(tab_hours.date, 7, 2) || 'T' || PRINTF('%02d',tab_hours.hour) || ':00') as `start_hour`,
	tab_hours.startSteps as found_steps,
	-- (tab_hours.endSteps - tab_hours.startSteps) as steps_in_hour,
	tab_diff.step_difference
FROM {{ ref('paseo_steps') }} tab_hours
INNER JOIN (SELECT
		`_id`,
		step_difference
	FROM temp_hour_diffs
	WHERE step_difference != 0) tab_diff
ON tab_hours._id = tab_diff._id
ORDER BY tab_hours.date, tab_hours.hour;
