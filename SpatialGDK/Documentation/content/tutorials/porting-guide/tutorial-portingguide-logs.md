<%(TOC)%>

# Port your project to SpatialOS

### Logs

You can find Spatial log files for your local deployments in `<ProjectRoot>\spatial\logs\`.  

- `spatial_<datetime>.log` contains all of the logs printed to your terminal during local deployments. - There are also timestamped folders here which contain additional logs:
  1. `<ProjectRoot>\spatial\logs\workers\` contain managed worker logs which are the workers started by SpatialOS, specified in your [launch configuration]({{urlRoot}}/content/glossary#launch-configuration).
  2. `<ProjectRoot>\spatial\logs\runtime.log` contains the logs printed by the SpatialOS Runtime. These are the services required for SpatialOS to run a local deployment.  

If you require additional debugging logs you can run `spatial local launch` with the flag `--log_level=debug`.

### Modify the default behavior

You can modify some of the GDK settings from the Unreal Editor toolbar at **Edit** > **Project Settings** >**SpatialOS Unreal GDK** > **Toolbar**.
You can change:

- the snapshot file's filename and location
- the launch configuration

**Next steps:** </br>

If you haven't already, check out the [multiserver offloading tutorial]({{urlRoot}}/content/tutorials/offloading-tutorial/offloading-intro).  

Also check out the documentation on [cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs), [handover]({{urlRoot}}/content/actor-handover) and [Singleton Actors]({{urlRoot}}/content/singleton-actors).

<br/>------------<br/>
_2020-01-17 Page updated with editorial review: suggest the multiserver offloading tutorial as a next step_<br/>
_2019-10-18 Page updated with editorial review: removed link to deprecated tutorial - multiple deployments for session-based games_<br/>
_2019-07-16 Page updated with editorial review_<br/>
