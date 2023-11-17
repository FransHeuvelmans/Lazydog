SELECT
	ds.day_date,
	ds.move_minutes,
	ds.distance_meter,
	ds.heart_point,
	ds.heart_minutes,
	ds.step_count
FROM {{ source('google_fit', 'g_fit_daily_summaries') }} as ds
