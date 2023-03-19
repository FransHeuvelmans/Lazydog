-- First we move the tables to more meaningful names

ALTER TABLE rBodyMass RENAME TO healthkit_BodyMass;
ALTER TABLE rDistanceWalkingRunning RENAME TO healthkit_DistanceWalkingRunning;
ALTER TABLE rFlightsClimbed RENAME TO healthkit_FlightsClimbed;
ALTER TABLE rHeight RENAME TO healthkit_Height;
ALTER TABLE rSleepAnalysis RENAME TO healthkit_SleepAnalysis;
ALTER TABLE rStepCount RENAME TO healthkit_StepCount;

-- Make sure tool_info is there and updated
CREATE TABLE IF NOT EXISTS tool_info (
	tool_name TEXT NOT NULL PRIMARY KEY,
  	latest_version TEXT NOT NULL,
  	creation_time TEXT NOT NULL,
	latest_import TEXT NOT NULL
  );

-- Update latest_version here
INSERT INTO tool_info (tool_name, latest_version, creation_time, latest_import) 
			VALUES ('healthkit-to-sqlite', '1.0.1', datetime('now'), datetime('now'));