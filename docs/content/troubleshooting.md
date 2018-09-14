<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# Unreal GDK Troubleshooting/FAQ



**Q:** I’m getting the error `"Could not find definition for module 'SpatialGDK' (referenced via Target -> <ProjectName>.Build.cs)"` when building my project.

**A:** You need to setup symlinks to the GDK as per [step 3 here]({{urlRoot}}/setup-and-installing#building).

------

**Q:** I've set up my Actor for replication according to Unreal Engine’s [documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), but my Actor does not replicate.

**A:** There could be a few different reasons for this. The list below provides some of the most common ones, ordered by likelihood:
1. It's easy to forget to generate the [type bindings]({{urlRoot}}/content/interop) for your replicated Actor. Make sure you run the Interop Code Generator and rebuild your project with these type bindings setup.
1. As per Unreal Engine’s [replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), your Actor needs to be created on the server-worker before it can replicate to the client-workers.
1. Ensure that your call to `SpawnActor` is happening on your server-worker.
Validate that the SpatialOS entity that represents your Actor appears in the Inspector. If it doesn't, then it's likely that it's not marked up for replication correctly.
1. Mark your Actor for replication as per [Unreal Engine’s Actor replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors). You can validate that your Actor is replicated in `USpatialNetDriver::ServerReplicateActors`.
1. Validate that you receive an `AddEntityOp` for the entity representing your Actor in the `USpatialnteropPipelineBlock` and that your entity is spawned in `USpatialnteropPipelineBlock::CreateEntity`.

------

**Q:** I've moved my game over to the SpatialOS Unreal GDK and am getting a crash in `UEngine::TickWorldTravel` when launching a PIE instance of my game.

**A:** Ensure that you have set up your Game Instance Class in your project settings (**Edit** > **Project Settings** > **Project** > **Maps & Modes** > **Game Instance Class**) to point to either `SpatialGameInstance` or a game instance that inherits from `USpatialGameInstance`.

------

**Q:** Can I change between Unreal Engine’s networking stack and the SpatialOS Unreal GDK networking stack?

**A:** Yes you can! In Visual Studio open the Properties window for you game project (right-click on your project and select **Properties**). Select the **Debugging** tab and, in the **Command Arguments** field, add the following to the to the end of the existing command line arguments:
`-NetDriverOverrides=/Script/OnlineSubsystemUtils.IpNetDriver`. When you launch your project, it'll use the default Unreal networking stack. Just remove the line if you want to go back to the SpatialOS Unreal GDK networking stack.

------

**Q:** I’m getting the following compilation error when building the GDK: `Error C2248: FRepLayout::Cmds': cannot access private member declared in class 'FRepLayout`.

**A:** You're building against an unsupported version of Unreal Engine. Make sure you're targeting the fork of Unreal Engine that the GDK requires. See the [setup guide]({{urlRoot}}/setup-and-installing#building) for more details.

------

**Q:** When running the Interop Code Generator, I’m getting the following error: `Error: Could not move generated interop files during the diff-copy stage`.

**A:** Due to the [known issue]({{urlRoot}}/known-issues) with piped input from child processes, it is not possible to get the exact error message. However, the most likely cause of the error, especially when working with Perforce, is that some of the files in the output folder (see the second argument in the error message) are marked as read-only. If that is the cause, you can resolve the error by changing all files to be writable.

Note that you may see similar errors if the same issue applies to the schema generation or the legacy SDK codegen output within the interop code generation step.

------

**Q:** My game uses reliable multicast RPCs - why does the SpatialOS Unreal GDK not support these?

**A:** The underlying implementation of multicast RPCs uses SpatialOS [events](https://docs.improbable.io/reference/latest/shared/glossary#event) (SpatialOS documentation). SpatialOS events can only be sent unreliably. Additionally, the cost of a multicast RPC scales with the number of client-workers present in a deployment, which means they can get very expensive. A better approach would be to send RPCs to only the workers that are close to the broadcasting worker.

------

**Q:** I’m getting the error `Unrecognized type 'UCommander' - type must be a UCLASS, USTRUCT or UENUM` when building my project in Visual Studio.

**A:** This is most likely due to not having run the legacy SDK code generation. Try running `Scripts/Codegen.bat` at least once. This should generate the required files for continuing the build process.

------

**Q:** When I try to start SpatialOS from the toolbar plugin, I get the following notification:
`[improbable.worker.assembly.WorkerAssemblyProvider] The worker assembly does not contain any worker configurations. No workers will be able to connect to this deployment. Unless you specifically intended to start a deployment without any workers, please make sure your assembly was built correctly, and in case of a cloud deployment, also make sure it was uploaded correctly.`

**A:** This is an indication that you haven’t built the worker configurations for the server-workers and client-workers. You can fix this by running the `Scripts/BuildWorkerConfig.bat` script. This generates the worker configs which allow your workers to connect to the local instance of SpatialOS.

------

**Q:** I'm getting a check failure in one of my typebinding classes:
```
check(!Value->IsFullNameStableForNetworking())
```

**A:** This is mostly likely caused by attempting to replicate a stably-named actor. This can occur if you place an Actor within a level and mark it for replication. We don't currently support this combination, although we will soon. Until then please spawn your actor dynamically at runtime.

------

**Q:** When I build my project, I get the following error: `Unknown class specifier 'SpatialType'`.

**A:** `SpatialType` is a new class specifier for tagging of classes to replicate to SpatialOS. To stop the error message, rebuild the `Unreal Header Tool` using the `Development` configuration.