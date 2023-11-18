select
	ds.day_date,
	ds.heart_point,
	w.average_kilo
from {{ source('google_fit', 'g_fit_daily_summaries') }} as ds
inner join {{ source('google_fit', 'g_fit_weights') }} as w
on ds.weight_id = w.weight_id