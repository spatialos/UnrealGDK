### Logs

You can find Spatial log files for your local deployments in `<ProjectRoot>\spatial\logs\`.  

- `spatial_<datetime>.log` contains all of the logs printed to your terminal during local deployments. - There are also timestamped folders here which contain additional logs:
  1. `<ProjectRoot>\spatial\logs\workers\` contain managed worker logs which are the workers started by SpatialOS, specified in your [launch configuration]({{urlRoot}}/content/glossary#launch-configuration).
  2. `<ProjectRoot>\spatial\logs\runtime.log` contains the logs printed by the SpatialOS Runtime. These are the services required for SpatialOS to run a local deployment.  

If you require additional debugging logs you can run `spatial local launch` with the flag `--log_level=debug`.

</br>

### Modify the default behavior

You can modify some of the GDK settings from the Unreal Editor toolbar at **Edit** > **Project Settings** >**SpatialOS Unreal GDK** > **Toolbar**.
You can change:

- the snapshot file's filename and location
- the launch configuration

</br>
**Next steps:** </br>

If you haven't already, check out:

- The tutorial on [multiple deployments for session-based games]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-intro).
- The Multiserver Shooter tutorial to learn how to implement [cross-server interactions]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro).  

Also check out the documentation on [cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs), [handover]({{urlRoot}}/content/actor-handover) and [Singleton Actors]({{urlRoot}}/content/singleton-actors).



<br/>------</br>
_2019-04-11 Added Shooter Game as a reference project with limited editorial review._