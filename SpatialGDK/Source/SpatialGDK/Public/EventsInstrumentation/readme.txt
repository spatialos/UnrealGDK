- All processing, logging and event definition lives in this folder
- Logger and processor are currently initialised and owned by SpatialNetDriver
- Logging directory is hard-coded and needs to be set in SpatialNetDriver.cpp, find "{log directory}"

- Events just need to be USTRUCT() derived from FLogEvent
- All fields inside these structs marked as UPROPERTY() will be automatically converted to json fields in the log files

- All events currently hooked in SpatialSender / SpatialReceiver
- Some data inconsistencies (eg. odd entity ids, missing actor types) particularly when starting / stoping the deployment

- For cloud, believe we should be able to get the log directory from launch args, and create a directory within to store the event logs
- Might want to turn single log file into rotating log file to avoid massive file sizes
- Could selectively disable certain events to keep sizes down if needed