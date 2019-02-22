<%(TOC)%>
# Troubleshooting

#### Q:
I've set up my Actor for replication according to Unreal Engine’s [documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), but my Actor does not replicate.

#### A:
There could be a few different reasons for this. The list below provides some of the most common ones, ordered by likelihood:

1. It's easy to forget to generate the schema for your replicated Actor. Make sure you run the Schema Generator before launching your project.

1. As per Unreal Engine’s [replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), your Actor needs to be created on the server-worker before it can replicate to the client-workers.

1. Ensure that your call to `SpawnActor` is happening on your server-worker.<br/>
Validate that the SpatialOS entity that represents your Actor appears in the Inspector. If it doesn't, then it's likely that it's not marked up for replication correctly.

1. Mark your Actor for replication as per [Unreal Engine’s Actor replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors). You can validate that your Actor is replicated in `USpatialNetDriver::ServerReplicateActors`.

1. Validate that you receive an `WORKER_OP_TYPE_ADD_ENTITY` for the entity representing your Actor in the `USpatialView::ProcessOps` and that your entity is spawned in `USpatialReceiver::CreateActor`.

<br/>
-----

#### Q: 
I've moved my game over to the SpatialOS GDK for Unreal and am getting a crash in `UEngine::TickWorldTravel` when launching a PIE instance of my game.

#### A:
Ensure that you have set up your Game Instance Class in your project settings (from the Unreal Editor, naviagte to: **Edit** > **Project Settings** > **Project** > **Maps & Modes** > **Game Instance Class**) to point to either `SpatialGameInstance` or a game instance that inherits from `USpatialGameInstance`.
<br/>
-----

#### Q:
Can I change between Unreal Engine’s networking stack and the SpatialOS GDK for Unreal networking stack?

#### A:
Yes you can! In the `Unreal Editor`, select the `Play` dropdown from the toolbar, and toggle the `Spatial Networking` checkbox to switch between the two networking stacks.
<br/>
-----

#### Q: 
I’m getting the following compilation error when building the GDK: `Error C2248: FRepLayout::Cmds': cannot access private member declared in class 'FRepLayout`.

#### A: 
You're building against an unsupported version of Unreal Engine. Make sure you're targeting the fork of Unreal Engine that the GDK requires. See the [setup guide]({{urlRoot}}/content/get-started/dependencies) for more details.
<br/>
-----

### Q: 
My game uses reliable multicast RPCs - why does the SpatialOS GDK for Unreal not support these?

#### A: 
The underlying implementation of multicast RPCs uses SpatialOS [events](https://docs.improbable.io/reference/latest/shared/glossary#event). SpatialOS events can only be sent unreliably. Additionally, the cost of a multicast RPC scales with the number of client-workers present in a deployment, which means they can get very expensive. A better approach would be to send RPCs to only the workers that are close to the broadcasting worker.
<br/>
-----

#### Q: 
When I build my project, I get the following error: `Unknown class specifier 'SpatialType'`.

#### A: 
`SpatialType` is a new class specifier for tagging of classes to replicate to SpatialOS. This message likely means that you have not built our UE4 fork, or pointed your project to use it. Follow the steps [here]({{urlRoot}}/content/get-started/introduction) to ensure the GDK is set up correctly.
<br/>
-----

#### Q:  
When I launch my SpatialOS deployment, I receive error messages similar to: `uses component ID 100005 which conflicts with components defined elsewhere.`

#### A:  
This means you were using the GDK since pre-alpha. To fix the issue, delete the contents of your `spatial/schema` folder, run `Setup.bat` again in the GDK folder, and generate the schemas again. You may also need to update your streaming queries in `spatialos.json`. Refer to our [StarterProject repo](https://github.com/spatialos/UnrealGDKStarterProject) to see an example.
<br/>
-----

#### Q:
When I run `Setup.bat`, I get the following error: `error MSB3644: The reference assemblies for framework ".NETFramework,Version=4.5" were not found`.

(The full error message looks like:
`C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\bin\Microsoft.Common.CurrentVersion.targets(1179,5): error MSB3644: The reference assemblies for framework ".NETFramework,Version=4.5" were not found. To resolve this, install the SDK or Targeting Pack for this framework version or retarget your application to a version of the framework for which you have the SDK or Targeting Pack installed. Note that assemblies will be resolved from the Global Assembly Cache (GAC) and will be used in place of reference assemblies. Therefore your assembly may not be correctly targeted for the framework you intend. [C:\MyProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Programs\Improbable.Unreal.Scripts\Build\Build.csproj]`)

#### A:
When you install Visual Studio as part of the [GDK dependencies]({{urlRoot}}/content/get-started/dependencies) set up step, ensure you have selected the **Universal Windows Platform development** workload. This workload includes .NET Framework 4.5. If you have already installed Visual Studio, you can add the missing workload by running the Visual Studio installer and clicking **Modify** on your existing Visual Studio Installation. 
<br/>
-----
