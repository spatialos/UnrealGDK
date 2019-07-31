# Troubleshooting

### Q: I've set up my Actor for replication according to Unreal Engine’s [documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), but my Actor does not replicate.

There could be a few different reasons for this. The list below provides some of the most common ones, ordered by likelihood:

1. It's easy to forget to generate the schema for your replicated Actor. Make sure you [generate schema]({{urlRoot}}/content/how-to-use-schema) before launching your project.

2. As described in Unreal's [replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), your game needs to create an Actor on server-worker instances before it can replicate to client-worker instances.

3. Ensure that your call to `SpawnActor` is happening on your server-worker instance.<br/>
Validate that the entity that represents your Actor appears in the Inspector. If it doesn't, then it's likely that it's not marked up for replication correctly.

1. Mark your Actor for replication as described in [Unreal's Actor replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors). You can validate that your Actor is replicated in `USpatialNetDriver::ServerReplicateActors`.

1. Validate that you receive an `WORKER_OP_TYPE_ADD_ENTITY` for the entity representing your Actor in the `USpatialView::ProcessOps` and that your entity is spawned in `USpatialReceiver::CreateActor`.

------

### Q: I've moved my game over to the SpatialOS GDK for Unreal and am getting a crash in `UEngine::TickWorldTravel` when launching a PIE instance of my game.

Ensure that you have set up your Game Instance Class in your project settings (from the Unreal Editor, naviagte to: **Edit** > **Project Settings** > **Project** > **Maps & Modes** > **Game Instance Class**) to point to either `SpatialGameInstance` or a game instance that inherits from `USpatialGameInstance`.

------

### Q:Can I change between Unreal Engine’s networking stack and the SpatialOS GDK for Unreal networking stack?

Yes you can! In the `Unreal Editor`, select the `Play` dropdown from the toolbar, and toggle the `Spatial Networking` checkbox to switch between the two networking stacks.

------

### Q: I’m getting the following compilation error when building the GDK: `Error C2248: FRepLayout::Cmds': cannot access private member declared in class 'FRepLayout`.

You're building against an unsupported version of Unreal Engine. Make sure you're targeting the fork of Unreal Engine that the GDK requires. See the [setup guide]({{urlRoot}}/content/get-started/dependencies) for more details.

------

### Q: My game uses reliable multicast RPCs - why does the SpatialOS GDK for Unreal not support these?

The underlying implementation of multicast RPCs uses SpatialOS [events](https://docs.improbable.io/reference/latest/shared/glossary#event). SpatialOS events can only be sent unreliably. Additionally, the cost of a multicast RPC scales with the number of client-worker instances present in a deployment, which means they can get very expensive. A better approach would be to send RPCs to only the worker instances that are close to the broadcasting worker instance.

------

### Q: When I build my project, I get the following error: `Unknown class specifier 'SpatialType'`.

`SpatialType` is a new class specifier for tagging of classes to replicate to SpatialOS. This message likely means that you have not built our UE4 fork, or pointed your project to use it. Follow the steps [here]({{urlRoot}}/content/get-started/introduction) to ensure the GDK is set up correctly.

------

### Q: When I launch my SpatialOS deployment, I receive error messages similar to: `uses component ID 100005 which conflicts with components defined elsewhere.`

This means you were using the GDK since pre-alpha. To fix the issue, delete the contents of your `spatial/schema` folder, run `Setup.bat` again in the GDK folder, and generate the schemas again. You may also need to update your streaming queries in `spatialos.json`. Refer to our [Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) to see an example.

------

### Q: When I run `Setup.bat`, I get the following error: `error MSB3644: The reference assemblies for framework ".NETFramework,Version=4.5" were not found`.

(The full error message looks like:
`C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\bin\Microsoft.Common.CurrentVersion.targets(1179,5): error MSB3644: The reference assemblies for framework ".NETFramework,Version=4.5" were not found. To resolve this, install the SDK or Targeting Pack for this framework version or retarget your application to a version of the framework for which you have the SDK or Targeting Pack installed. Note that assemblies will be resolved from the Global Assembly Cache (GAC) and will be used in place of reference assemblies. Therefore your assembly may not be correctly targeted for the framework you intend. [C:\MyProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Programs\Improbable.Unreal.Scripts\Build\Build.csproj]`)

When you install Visual Studio as part of the [GDK dependencies]({{urlRoot}}/content/get-started/dependencies) set up step, ensure you have selected the **Universal Windows Platform development** workload. This workload includes .NET Framework 4.5. If you have already installed Visual Studio, you can add the missing workload by running the Visual Studio installer and clicking **Modify** on your existing Visual Studio Installation.


### Q: My worker instances are being disconnected from the SpatialOS Runtime unexpectedly while debugging locally.

(You see log messages similar to these while debugging your game: `LogNet: UNetConnection::Cleanup: Closing open connection. [UNetConnection] RemoteAddr: , Name: SpatialNetConnection_14, Driver: GameNetDriver SpatialNetDriver_17, IsServer: YES, PC: BP_PlayerController_C_0, Owner: BP_PlayerController_C_0, UniqueId: …)`.

This is caused by a [known issue](https://github.com/spatialos/UnrealGDK/issues/940) in the GDK's connection “heartbeating” logic that disconnects worker instances that have stopped responding. 

To work around the issue:

1. In the Unreal Editor, navigate to **Edit** > **Project Settings** > **Runtime Settings**.
1. Update the value of **Heartbeat Timeout** from the default 10 seconds to a much larger value that you’re unlikely to reach while debugging, such as 10000.
1. Navigate to **Edit** > **Project Settings** > **Editor Settings**.
1. Make sure the **Legacy Flags** section (within the **Launch** section) includes an entry to set [`bridge_qos_max_timeout`](https://docs.improbable.io/reference/latest/shared/project-layout/launch-config#legacy-flags) ) to `0`. 

    Once you’ve finished debugging your game and want to deploy it locally or in the cloud, make sure you set these values back to their defaults so that failing worker instances are correctly disconnected from the SpatialOS Runtime.

<br/>
<br/>------------<br/>
_2019-07-02 Page updated with limited editorial review: added debug workaround_