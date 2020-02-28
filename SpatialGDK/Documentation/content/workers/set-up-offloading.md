<%(TOC)%>

# Set up your game feature for offloading

To set up your game feature for offloading, in addition to configuring your Unreal game server, you also need to start an Unreal process for your offloaded worker types, both locally in PIE and in the cloud. In particular, you specify the number of offloaded workers to start and their attributes in the Unreal Editor, and configure Actor groups to set the authority configuration for the Actors.

Before you start, review the following offloading workflows. You can check each step for detailed information:

1. Create a [worker configuration file]({{urlRoot}}/content/glossary#worker-configuration-file): You create a worker configuration file for the new offloaded worker type.

    **Note**: If you launch a local deployment through PIE within the editor, the Unreal GDK adds the worker configuration file for your new offloaded worker type, so you can skip this step.
2. Specify the launch configuration
    - For local deployment: Specify the load balancing strategy for the new offloaded worker and number of worker instances to be launched.
    - For cloud deployment: Edit the auto-generated configuration file so that the Runtime requests the start of managed workers.
3. Configure Actor groups: Create Actor groups, assign Actor classes to a group, and then assign each group to a server-worker type.

    **Notes**:
    - By default, when an Actor type does not have a mapping to an Actor group, its authority is assigned to the main Unreal server-worker type.
    - You must configure actor groups on a class level rather than on an instance level.
4. Generate a snapshot: Validate the snapshot is set up correctly.
5. Test changes locally: Launch a local deployment.
6. Test changes in the cloud: Package the worker type and create a cloud deployment.

## 1. Create a worker configuration file

<%(Callout type="info" message="You can skip this step if you launch a local deployment through PIE within the editor because the Unreal GDK adds the worker configuration file for your new offloaded worker type. However, when you launch a cloud deployment, but no configuration file for the new offloaded worker type is available, you must create a worker configuration file for it.")%>

1. Open File Explorer and navigate to `<ProjectRoot>GameOne/spatial/workers/unreal`.
2. Make a copy of the `spatialos.UnrealWorker.worker.json` file and rename it to `spatialos.<YourWorkerName>.worker.json`, where `YourWorkerName` is the name of the new offloaded worker type.
3. Open the `spatialos.<YourWorkerName>.worker.json`. In the `worker_attribute_set` attributes array, replace the reference to `UnrealWorker` with the name of your offloaded worker.

    > **Note**: Don’t rename the `UnrealWorker@Linux.zip`, which is used to launch managed workers on Linux.

## 2. Specify the launch configuration

### For local deployment

After you create a new worker configuration file, you must configure the default launch configuration with the correct load balancing settings and specify what server worker types to be in the local deployment. By default, only one `UnrealWorker` is in the launch configuration file.

Complete the following steps to configure the launch configuration for each worker type:

1. Open your project in the Unreal Editor.
2. Select **Play** > **SpatialOS Settings** to display the Project Settings editor. 
3. Scroll down to **SpatialOS GDK for Unreal** and select **Editor Settings** to display the SpatialOS Editor Settings panel.
4. Go to **Launch** > **Launch configuration description** > **Server Workers** configuration section. 
5. Add the new load-balancing strategy for the new offloaded worker. The following screenshot shows an example of an `InteractionWorker` with a 1x1 rectangle grid load-balancing strategy:
    
    > **Note**: Specify the launch configuration only for server-worker types, and don’t specify a load balancing configuration for your game clients because this causes runtime startup errors.

    ![img]({{assetRoot}}assets/specify-launch-configuration.png)
6. After you define the load balancing strategy, in the **Instances to launch in editor** field, specify the number of instances of each worker type to be launched, as is shown in the screenshot above in blue rectangle.

### For cloud deployment

To specify the launch configuration for the cloud deployment, in addition to the steps that you complete for the local deployment, you must also take the following steps:

1. Create a copy of the auto-generated launch configuration from `...\Intermediate\Improbable\DefaultLaunchConfig,json` to the directory where you keep your launch configuration files (usually `spatial`). For more information, see [Auto-generated launch configuration file]({{urlRoot}}/content/unreal-editor-interface/toolbars#auto-generated-launch-config-for-pie-server-worker-types).
2. Open the new launch configuration file, and remove the `manual_worker_connection_only` option from all workers so that the Runtime requests the start of managed workers.

## 3. Configure Actor groups

Complete the following steps to configure Actor groups:

1. In the Unreal Editor, select **Play** > **SpatialOS Settings** to display the Project Settings editor.
2. Scroll down to **SpatialOS GDK for Unreal** and select **Runtime Settings** to display the SpatialOS Runtime Settings panel.
3. In the **Offloading** section, select the **Enable offloading** check box. Then, you can add new Actor groups.
4. From **Actor Groups**, click the plus sign (+) to add a new entry to the **Actor Groups** list.
5. Specify the name for the Actor group in the field.
6. From the **Owning Worker Type** drop-down list, select the server worker type that has authority over this Actor group.
7. Add one or more classes that you assign to the Actor group. The following screenshot shows the `AIController` class is added to the AI Actor group.

    ![img]({{assetRoot}}assets/configure-actor-groups.png)

## 4. Generate a snapshot

After you configure the worker types, you must generate a new snapshot first before the deployment. This is because the snapshot generator sets the authority and interest configurations for GDK managed entities based on the worker types that you launch in the deployment.

Complete the following steps to validate that the snapshot is set up correctly:

1. Launch a local deployment.
2. Find the `GlobalStateManager` entity in the snapshot.
3. From the `Read A.C.L. Attribute` section in the Inspector, you should find all the server worker types that you specified in the launch configuration. If any of your server worker types is missing, your snapshot is out of date.

The following screenshot shows the read ACL of a GSM entity configured with read access to the following server-worker types:

- UnrealWorker
- AIWorker
- CrashBotWorker

    ![img]({{assetRoot}}assets/snapshot-validation.png)

## 5. Test changes locally

You launch a local deployment in the Unreal Editor in the same way as you did with the standard Unreal GDK local deployment.

For more information, see the [Local deployment workflow]({{urlRoot}}/content/local-deployment-workflow).

## 6. Test changes in the cloud

### Package the worker type

Offloaded worker uses the same binary as the other server worker, for example, `UnrealWorker@Linux.zip`. The only difference is that the offloaded worker type in your worker configuration is passed in as the worker type [command-line argument]({{urlRoot}}/content/command-line-arguments).

For more information about building server-worker assembly for the cloud, see the [Cloud deployment workflow]({{urlRoot}}/content/cloud-deployment-workflow).

### Create a cloud deployment

You create a cloud deployment with offloaded workers in the same way as you did with the standard Unreal GDK cloud deployment.

For more information, see the [Cloud deployment workflow]({{urlRoot}}/content/cloud-deployment-workflow).

<br/>------------<br/>
_2019-07-30 Page added with limited editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------