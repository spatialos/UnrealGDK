<%(TOC)%>
# GDK Runtime Settings

| Item | Category | Description | Example | Notes |
|------|----------|-------------|---------|-------|
| Initial entity ID reservation count | Entity Pool | The number of entity IDs to be reserved when the entity pool is first started. | 3000 | |
| Pool refresh minimum threshold | Entity Pool | The minimum number of entity IDs available in the pool before a new batch is reserved | 1000 | |
| Refresh count | Entity Pool | The number of entity IDs reserved when the minimum threshold is reached | 2000 | |
| Heartbeat interval (seconds) | Heartbeat | Time between heartbeat events sent from clients to notify the servers they are still connected. | 2.0 | |
| Heartbeat timeout (seconds) | Heartbeat | Time that should pass since the last heartbeat event received to decide a client has disconnected | 10.0 | |
| Actor replication rate limit | Replication | Limit the number of actors which are replicated per tick to the number specified. This acts as a hard limit to the number of actors per frame but nothing else. It's recommended to set this value to around 100~ (experimentation recommended). If set to 0, SpatialOS will replicate every actor per frame (unbounded) and so large worlds will experience slowdown server-side and client-side. Use `stat SpatialNet` in editor builds to find the number of calls to `ReplicateActor` and use this to inform the rate limit setting. | 100 | |
| SpatialOS network update rate | Replication | Rate at which updates are sent to SpatialOS and processed from SpatialOS. | 30.0 | |
| Query Based Interest enabled | Query Based Interest | Query Based Interest is required for level streaming and the AlwaysInterested UPROPERTY specifier to be supported when using spatial networking, however comes at a performance cost for larger-scale projects. | true | |
| Position update frequency | Spatial OS Position Updates | Frequency for updating an Actor's SpatialOS Position. Updating position should have a low update rate since it is expensive. | 1.0 | |
| Position distance threshold | Spatial OS Position Updates | Threshold an Actor needs to move before its SpatialOS Position is updated. | 100.0 | |
