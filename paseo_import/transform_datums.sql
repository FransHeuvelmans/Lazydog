---- These are the raw sql for sqlite transformation steps
--- Get exact steps and hours better formatted
CREATE TABLE paseo_tracked_steps AS
SELECT 
	_id as source_id,
	datetime(substr(`date`, 1, 4) || '-' || substr(`date`, 5, 2) || '-' || substr(`date`, 7, 2) || 'T' || PRINTF('%02d',`hour`) || ':00') as `start_hour`,
	(endSteps - startSteps) as steps_in_hour
FROM hours;


-- Find when the steps suddenly dropped.
CREATE TEMPORARY TABLE temp_hour_diffs AS
SELECT 
	`_id`,
    -- Difference since last day
	(startSteps - LAG(startSteps, 1) OVER (ORDER BY `date`, `hour`)) AS step_difference
FROM hours
ORDER BY `date`, `hour`;


-- Steps which were not tracked by Paseo but reported to Paseo by Android
CREATE TABLE paseo_lost_steps AS
SELECT 
	tab_hours._id as source_id,
    datetime(substr(tab_hours.date, 1, 4) || '-' || substr(tab_hours.date, 5, 2) || '-' || substr(tab_hours.date, 7, 2) || 'T' || PRINTF('%02d',tab_hours.hour) || ':00') as `start_hour`,
	tab_hours.startSteps as found_steps,
	(tab_hours.endSteps - tab_hours.startSteps) as steps_in_hour,
	tab_diff.step_difference
FROM hours tab_hours
INNER JOIN (SELECT
		`_id`,
		step_difference
	FROM temp.temp_hour_diffs
	WHERE step_difference < 0) tab_diff
ON tab_hours._id = tab_diff._id
ORDER BY tab_hours.date, tab_hours.hour;

ALTER TABLE `hours` RENAME TO paseo_hours;
ALTER TABLE `android_metadata` RENAME TO paseo_android_metadata;
ALTER TABLE `activityType` RENAME TO paseo_activity_type;

-- Make sure tool_info is there and updated
CREATE TABLE IF NOT EXISTS tool_info (
	tool_name TEXT NOT NULL PRIMARY KEY,
  	latest_version TEXT NOT NULL,
  	creation_time TEXT NOT NULL,
	latest_import TEXT NOT NULL
  );

-- Update latest_version here
INSERT INTO tool_info (tool_name, latest_version, creation_time, latest_import) 
			VALUES ('paseo-to-lazydog', '0.1.0', datetime('now'), datetime('now'));