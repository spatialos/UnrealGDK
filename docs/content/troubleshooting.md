## Unreal GDK Troubleshooting/FAQ

------

**Q:** I’m getting the error `"Could not find definition for module 'SpatialGDK' (referenced via Target -> <ProjectName>.Build.cs)"` when building my project.

**A:** You need to setup the symlink to the GDK as per [step 3 here](../setup-and-installing.md#building).

------

**Q:** I have set up my Actor for replication according to Unreal Engine’s [documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), but my Actor does not replicate.

**A:** There could be a few different causes to this. The list below provides some of the most common ones, ordered by likelihood:
1. It is easy to forget to generate the [type bindings](./interop.md) for your replicated actor, ensure that you run the interop codegeneration and rebuild your project with these type bindings setup.
1. Per Unreal Engine’s [replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors), your actor needs to be created on the server before it can replicate to the clients.
1. Ensure that your call to SpawnActor is happening on your server.
Validate that the SpatialOS entity that represents your Actor appears in the inspector. If does not then it is most likely not marked up for replication correctly.
1. Mark your actor for replication as per [Unreal Engine’s actor replication documentation](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors). You can validate that your actor is replicated in USpatialNetDriver::ServerReplicateActors.
1. Validate that you receive an AddEntityOp for the entity representing your actor in the USpatialnteropPipelineBlock and that your entity is spawned in USpatialnteropPipelineBlock::CreateEntity.

------

**Q:** I’ve moved my game over to the SpatialOS Unreal GDK and am getting a crash in `UEngine::TickWorldTravel` when launching the a PIE instance of my game.

**A:** Ensure that you have set up your Game Instance Class in your project settings(**Edit > Project Settings > Project > Maps & Modes > Game Instance Class**) to point to either SpatialGameInstance or a game instance that inherits from USpatialGameInstance.

------

**Q:** Can I change between Unreal Engine’s networking stack and the SpatialOS Unreal GDK network stack?

**A:** Yes you can! In Visual Studio open the properties window for you game project (right-click on your project and select properties). Select the debugging tab, in the Command Arguments field, add the following to the to the end of the existing commandline:
`-NetDriverOverrides=/Script/OnlineSubsystemUtils.IpNetDriver`. Launch your project and then it will use the default Unreal network stack. Just remove the line if you want to go back to the SpatialOS Unreal GDK network stack.

------

**Q:** I’m getting the following compilation error when building the GDK: `Error C2248: FRepLayout::Cmds': cannot access private member declared in class 'FRepLayout`.

**A:** You are building against an unsupported version of the Unreal Engine. Ensure that you are targeting the fork of the Unreal Engine required by the GDK, see the [setup guide](../setup-and-installing.md#building) for more details.

------

**Q:** When running interop code generation, I’m getting the following error: `Error: Could not move generated interop files during the diff-copy stage.`.

**A:** Due to the [known issue](http:://Link-to-known-issues-page) with piped input from child processes, it is not possible to get the exact error message. However, the most likely cause of the error, especially when working with perforce is that some of the files in the output folder (See the second argument in the error message) are marked as read only. If that is the cause, then the error will be resolved by changing all files to be writable.

Note that you may see similar errors if the same issue applies to the schema generation or the legacy sdk codegen output within the interop code generation step.

------

**Q:** My game uses reliable multicast RPCs, why are these not supported in the SpatialOS UnrealGDK?

**A:** The underlying implementation of multicast RPCs uses SpatialOS events. SpatialOS events can only be sent unreliably. Additionally the cost of an multicast RPC scales with the number of clients present in a deployment which means that they can get very expensive. A better approach would be to send RPCs to only the workers that are close to the broadcasting worker.

------

**Q:** I’m getting `Unrecognized type 'UCommander' - type must be a UCLASS, USTRUCT or UENUM` when building my project in Visual Studio.

**A:** This is most likely due to not having run the legacy SDK codegeneration. Try running `Scripts/Codegen.bat` at least once. This should generate the required files for continuing the build process.

------

**Q:** When I try to start SpatialOS from the toolbar plugin, I get the following notification:
`[improbable.worker.assembly.WorkerAssemblyProvider] The worker assembly does not contain any worker configurations. No workers will be able to connect to this deployment. Unless you specifically intended to start a deployment without any workers, please make sure your assembly was built correctly, and in case of a cloud deployment, also make sure it was uploaded correctly.`

**A:** This is an indication that you haven’t built the worker configurations for the server and the clients. This is fixed by running the `Scripts/BuildWorkerConfig.bat` script. This will generate the worker configs which would allow your workers to connect to the local instance of SpatialOS.

------

**Q:** I’m getting compilation errors in my typebindings about missing classes and/or namespaces.

**A:** Ensure that you have added the required headers to `DefaultEditorSpatialGDK.ini` as per the interop code generator documentation.
