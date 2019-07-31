# Changelog
All notable changes to the SpatialOS Game Development Kit for Unreal will be documented in this file.

The format of this Changelog is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased-`x.y.z`] - 2019-xx-xx

## [`0.6.0`] - 2019-07-31

### Breaking Changes:
* You must [re-build](https://docs.improbable.io/unreal/alpha/content/get-started/example-project/exampleproject-setup#step-4-build-the-dependencies-and-launch-the-project) your [Example Project](https://github.com/spatialos/UnrealGDKExampleProject) if you're upgrading it to `0.6.0`.
* This is the last GDK version to support Unreal Engine 4.20. You will need to upgrade your project to use Unreal Engine 4.22 (`4.22-SpatialOSUnrealGDK-release`) in order to continue receiving GDK releases and support.

### New Known Issues:
- Workers will sometimes not gain authority when quickly reconnecting to an existing deployment, resulting in a failure to spawn or simulate. When using the editor if you Play - Stop - Play in quick succession you can sometimes fail to launch correctly.

### Features:
- The GDK now uses SpatialOS `13.8.1`.
- Dynamic components are now supported. You can now dynamically attach and remove replicated subobjects to Actors.
- Local deployment startup time has been significantly reduced.
- Local deployments now start automatically when you select `Play`. This means you no longer need to select `Start` in the GDK toolbar before you select `Play` in the Unreal toolbar.
- If your schema has changed during a local deployment, the next time you select `Play` the deployment will automatically restart.
- Local deployments no longer run in a seperate Command Prompt. Logs from these deployments are now found in the Unreal Editor's Output Log.
- SpatialOS Runtime logs can now be found at `<GameRoot>\spatial\logs\localdeployment\<timestamp>\runtime.log`.
- An option to `Show spatial service button` has been added to the SpatialOS Settings menu. This button can be useful when debugging.
- Every time you open a GDK project in the Unreal Editor, 'spatial service' will be restarted. This ensures the service is always running in the correct SpatialOS project. You can disable this auto start feature via the new SpatialOS setting `Auto-start local deployment`.
- Added external schema code-generation tool for [non-Unreal server-worker types]({{urlRoot}}/content/non-unreal-server-worker-types). If you create non-Unreal server-worker types using the [SpatialOS Worker SDK](https://docs.improbable.io/reference/13.8/shared/sdks-and-data-overview) outside of the GDK and your Unreal Engine, you manually create [schema]({{urlRoot}/content/glossary#schema). Use the new [helper-script]({{urlRoot}}/content/helper-scripts) to generate Unreal code from manually-created schema; it enables your Unreal game code to interoperate with non-Unreal server-worker types.
- Added simulated player tools, which will allow you to create logic to simulate the behavior of real players. Simulated players can be used to scale test your game to hundreds of players. Simulated players can be launched locally as part of your development flow for quick iteration, as well as part of a separate "simulated player deployment" to connect a configurable amount of simulated players to your running game deployment.
- Factored out writing of Linux worker start scripts into a library, and added a standalone `WriteLinuxScript.exe` to _just_ write the launch script (for use in custom build pipelines).
- Added temporary MaxNetCullDistanceSquared to SpatialGDKSettings to prevent excessive net cull distances impacting runtime performance. Set to 0 to disable.
- Added `OnWorkerFlagsUpdated`, a delegate that can be directly bound to in C++. To bind via blueprints you can use the blueprint callable functions `BindToOnWorkerFlagsUpdated` and `UnbindToOnWorkerFlagsUpdated`. You can use `OnWorkerFlagsUpdated` to register when worker flag updates are received, which allows you to tweak values at deployment runtime.
- RPC frequency and payload size can now be tracked using console commands: `SpatialStartRPCMetrics` to start recording RPCs and `SpatialStopRPCMetrics` to stop recording and log the collected information. Using these commands will also start/stop RPC tracking on the server.
- Spatial now respects `bAlwaysRelevant` and clients will always checkout Actors that have `bAlwaysRelevant` set to true.

### Bug fixes:
- The `improbable` namespace has been renamed to `SpatialGDK`. This prevents namespace conflicts with the C++ SDK.
- Disconnected players no longer remain on the server until they time out if the client was shut down manually.
- Fixed support for relative paths as the engine association in your games .uproject file.
- RPCs on `NotSpatial` types are no longer queued forever and are now dropped instead.
- Fixed issue where an Actor's Spatial position was not updated if it had an owner that was not replicated.
- BeginPlay is now only called with authority on startup actors once per deployment.
- Fixed null pointer dereference crash when trying to initiate a Spatial connection without an existing one.
- URL options are now properly sent through to the server when doing a ClientTravel.
- The correct error message is now printed when the SchemaDatabase is missing.
- `StartEditor.bat` is now generated correctly when you build a server worker outside of editor.
- Fixed an issue with logging errored blueprints after garbage collection which caused an invalid pointer crash.
- Removed the ability to configure snapshot save folder. Snapshots should always be saved to `<ProjectRoot>/spatial/snapshots`. This prevents an issue with absolute paths being checked in which can break snapshot generation.
- Introduced a new module, `SpatialGDKServices`, on which `SpatialGDK` and `SpatilGDKEditorToolbar` now depend. This resolves a previously cyclic dependency.
- RPCs called before entity creation are now queued in case they cannot yet be executed. Previously they were simply dropped. These RPCs are also included in RPC metrics.
- RPCs are now guaranteed to arrive in the same order for a given actor and all of its subobjects on single-server deployments. This matches native Unreal behavior.

## [`0.5.0-preview`](https://github.com/spatialos/UnrealGDK/releases/tag/0.5.0-preview) - 2019-06-25
- Prevented `Spatial GDK Content` from appearing under Content Browser in the editor, as the GDK plugin does not contain any game content.

### Breaking Changes:
- If you are using Unreal Engine 4.22, the AutomationTool and UnrealBuildTool now require [.NET 4.6.2](https://dotnet.microsoft.com/download/dotnet-framework/net462).

### New Known Issues:

### Features:
- Unreal Engine 4.22 is now supported. You can find the 4.22 verson of our engine fork [here](https://github.com/improbableio/UnrealEngine/tree/4.22-SpatialOSUnrealGDK-release).
- Setup.bat can now take a project path as an argument. This allows the UnrealGDK to be installed as an Engine Plugin, pass the project path as the first variable if you are running Setup.bat from UnrealEngine/Engine/Plugins.
- Removed the need for setting the `UNREAL_HOME` environment variable. The build and setup scripts will now use your project's engine association to find the Unreal Engine directory. If an association is not set they will search parent directories looking for the 'Engine' folder.
- Added the `ASpatialMetricsDisplay` class, which you can use to view UnrealWorker stats as an overlay on the client.
- Added the runtime option `bEnableHandover`, which you can use to toggle property handover when running in non-zoned deployments.
- Added the runtime option `bEnableMetricsDisplay`, which you can use to auto spawn `ASpatialMetricsDisplay`, which is used to remote debug server metrics.
- Added the runtime option `bBatchSpatialPositionUpdates`, which you can use to batch spatial position updates to the runtime.
- Started using the [schema_compiler tool](https://docs.improbable.io/reference/13.8/shared/schema/introduction#using-the-schema-compiler-directly) to generate [schema descriptors](https://docs.improbable.io/reference/13.8/shared/flexible-project-layout/build-process/schema-descriptor-build-process#schema-descriptor-introduction) rather than relying on 'spatial local launch' to do this.
- Changed Interest so that NetCullDistanceSquared is used to define the distance from a player that the actor type is *interesting to* the player. This replaces CheckoutRadius which defined the distance that an actor is *interested in* other types. Requires engine update to remove the CheckoutRadius property which is no longer used.
- Added ActorInterestComponent that can be used to define interest queries that are more complex than a radius around the player position.
- Enabled new Development Authentication Flow
- Added new "worker" entities which are created for each server worker in a deployment so they correctly receive interest in the global state manager.
- Added support for spawning actors with ACLs configured for offloading using actor groups.
- Removed the references to the `Number of servers` slider in the Play in editor drop-down menu. The number of each server worker type to launch in PIE is now specified within the launch configuration in the `Spatial GDK Editor Settings` settings tab.
- Added `SpatialWorkerId` which is set to the worker ID when the worker associated to the `UGameInstance` connects.
- Added `USpatialStatics` helper blueprint library exposing functions for checking if SpatialOS networking is enabled, whether offloading is enabled, and more SpatialOS related checks.


### Bug fixes:
- BeginPlay is not called with authority when checking out entities from Spatial.
- Launching SpatialOS would fail if there was a space in the full directory path.
- GenerateSchemaAndSnapshots commandlet no longer runs a full schema generation for each map.
- Reliable RPC checking no longer breaks compatibility between development and shipping builds.
- Fixed an issue with schema name collisions.
- Running Schema (Full Scan) now clears generated schema files first.
- [Singleton actor's](https://docs.improbable.io/unreal/latest/content/singleton-actors#singleton-actors) authority and state now resumes correctly when reconnecting servers to snapshot.
- Retrying reliable RPCs with `UObject` arguments that were destroyed before the RPC was retried no longer causes a crash.
- Fixed path naming issues in setup.sh
- Fixed an assert/crash in `SpatialMetricsDisplay` that occurred when reloading a snapshot.
- Added Singleton and SingletonManager to QBI constraints to fix issue preventing Test configuration builds from functioning correctly.
- Failing to `NetSerialize` a struct in spatial no longer causes a crash, it now prints a warning. This matches native Unreal behavior.
- Query response delegates now execute even if response status shows failure. This allows handlers to implement custom retry logic such as clients querying for the GSM.
- Fixed a crash where processing unreliable RPCs made assumption that the worker had authority over all entities in the SpatialOS op
- Ordering and reliability for single server RPCs on the same Actor are now guaranteed.

### External contributors:

In addition to all of the updates from Improbable, this release includes x improvements submitted by the incredible community of SpatialOS developers on GitHub! Thanks to these contributors:

* @cyberbibby

## [`0.4.2`](https://github.com/spatialos/UnrealGDK/releases/tag/0.4.2) - 2019-05-20

### New Known Issues:
- `BeginPlay()` is not called on all `WorldSettings` actors [#937](https://github.com/spatialos/UnrealGDK/issues/937)
- Replicated properties within `DEBUG` or `WITH_EDITORONLY_DATA` macros are not supported [#939](https://github.com/spatialos/UnrealGDK/issues/939)
- Client connections will be closed by the `ServerWorker` when using Blueprint or C++ breakpoints during play-in-editor sessions [#940](https://github.com/spatialos/UnrealGDK/issues/940)
- Clients that connect after a Startup Actor (with `bNetLoadOnClient = true`) will not delete the Actor [#941](https://github.com/spatialos/UnrealGDK/issues/941)
- Generating schema while asset manager is asynchronously loading causes editor to crash [#944](https://github.com/spatialos/UnrealGDK/issues/944)

### Bug fixes:
- Adjusted dispatcher tickrate to reduce latency
- GenerateSchemaAndSnapshots commandlet no longer runs a full schema generation for each map.
- Launching SpatialOS would fail if there was a space in the full directory path.
- Fixed an issue with schema name collisions.
- Schema generation now respects "Directories to never cook".
- The editor no longer crashes during schema generation when the database is readonly.
- Replicating `UInterfaceProperty` no longer causes crashes.

## [`0.4.1`](https://github.com/spatialos/UnrealGDK/releases/tag/0.4.1) - 2019-05-01

### Bug fixes:
- Fixed an issue where schema components were sometimes generated with incorrect component IDs.

## [`0.4.0`](https://github.com/spatialos/UnrealGDK/releases/tag/0.4.0) - 2019-04-30

### Features:
- The GDK now uses SpatialOS `13.6.2`.
- Added this Changelog
- Added an error when unsupported replicated gameplay abilities are found in schema generation.
- Demoted various logs to Verbose in SpatialSender and SpatialReceiver
- You can now use the Project Settings window to pass [command line flags](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-local-launch#spatial-local-launch) to local deployments launched from the GDK toolbar.
- You can now adjust the SpatialOS update frequency and the distance an action must move before we update its SpatialOS position.

### Bug fixes:
- The worker disconnection flow is now handled by `UEngine::OnNetworkFailure` rather than the existing `OnDisconnection` callback, which has been removed.
- Fix duplicated log messages in `spatial CLI` output when running in PIE.
- Fixed deserialization of strings from schema.
- Ensure that components added in blueprints are replicated.
- Fixed potential loading issue when attempting to load the SchemaDatabase asset.
- Add pragma once directive to header file.
- Schema files are now generated correctly for subobjects of the Blueprint classes.
- Fixed being unable to launch SpatialOS if project path had spaces in it.
- Editor no longer crashes when setting LogSpatialSender to Verbose.
- Server-workers quickly restarted in the editor will connect to runtime correctly.
- Game no longer crashes when connecting to Spatial with async loading thread suspended.

## [`0.3.0`](https://github.com/spatialos/UnrealGDK/releases/tag/0.3.0) - 2019-04-04

### New Known Issues:
- Enabling Query Based Interest is needed for level streaming support, but this might affect performance in certain scenarios and is currently being investigated.
- Replicated `TimelineComponents` are not supported.

For current known issues, please visit [this](https://docs.improbable.io/unreal/alpha/known-issues) docs page

### Features:
- The default connection protocol is now TCP.
- Query Based Interest is now supported as an opt-in feature.
- Level streaming is now supported. You must enable Query Based Interest checkbox in the Runtime Settings to use level streaming.
- The GDK Toolbar now recognises when a local deployment is running, and contextually displays start and stop buttons. - (@DW-Sebastien)
- Added interface support for Unreal Engine 4.21 `UNetConnection`. - (@GeorgeR)
- Unreliable RPCs are now implemented using events instead of commands. This resolves associated performance issues.
- The `delete dynamic entities` setting now works when used in conjunction with multiple processes.
- You can now determine the type of a SpatialOS worker from within the game instance.
- Entity IDs are now reserved in batches instead of individually. This accelerates the creation of SpatialOS entities.
- You can now serialize and deserialize component data defined in external schema (schema that is not-generated by the Unreal GDK). You can use this to send and receive data, and edit snapshots.
- Improved logging during RPCs.

### Bug fixes:
- The GDK now automatically compiles all dirty blueprints before generating schema.
- Attempting to load a class which is not present in the schema database now causes the game to quit instead of crashing the entire editor.
- `Actor::ReplicateSubobjects` is now called in the replication flow. This means that Subobjects are now replicated correctly.
- Schema generation is no longer fatally halted when blueprints fail to compile.
- `AActor::TornOff` is now called when a `TearOff` event is received. This is in-line with the native implementation.
- References to objects within streaming levels, that are resolved before the level has streamed in, no longer cause defective behavior on the client.
- Attempting to replicate a `NonSpatial` actor no longer causes a crash.
- The SpatialOS Launcher now launches the correct game client, even when `UnrealCEFSubProcess.exe` is present in the assembly.
- Duplicate startup-actors are no longer created when a server-worker reconnects to a deployment.
- `BeginPlay` is no-longer called authoritatively when a server-worker reconnects to a deployment.
- Fast Array Serialization now operates correctly in conjunction with `GameplayAbilitySystem`.
- Reference parameters for RPCs are now correctly supported.
- Clients now load the map specified by the global state manager, rather than loading the `GameDefaultMap` _before_ querying the global state manager.
- Automatically generated launch configurations for deployments with a prime numbers of server-workers are now generated with the correct number of rows and columns.
- Generating schema for a level blueprint no longer deletes schema that has been generated for other levels.
- Deleting recently created actors no longer causes crashes.
- Having multiple EventGraphs no longer causes incorrect RPCs to be called.
- `TimerManager`, which is used by SpatialOS networking, is no longer subject to time dilation in the `World` instance.
- Clients no longer crash after being assigned multiple players.
- `GetWorkerFlag` can now be called from C++ classes.
- Pathless mapname arguments are now supported by the GDK commandlet.
- When `NotifyBeginPlay` is called, `BeginPlay` is no longer called on actors before their `Role` is correctly set.
- Deployments containing multiple server-workers no longer fails to initialize properly when launched through PIE with the `use single process` option unchecked.

### External contributors:

In addition to all of the updates from Improbable, this release includes 2 improvements submitted by the incredible community of SpatialOS developers on GitHub! Thanks to these contributors:

* @DW-Sebastien
* @GeorgeR

## [`0.2.0`](https://github.com/spatialos/UnrealGDK/releases/tag/0.2.0) - 2019-02-26

Startup actors revamp is merged! Snapshots are now simpler. Many bugfixes.

### New Known Issues:
- A warning about an out of date net driver is printed at startup of clients and server.

For current known issues, please visit [this](https://docs.improbable.io/unreal/alpha/known-issues) docs page

### Features:
- Actors placed in the level are no longer saved to the snapshot. They are instead spawned dynamically at the start of the game. This should fix quite a few issues such as missing references, and non-replicated instanced data being incorrectly set
- Pass player name and login options in the login URL
- Server will identify clients that have been disconnected from Spatial and trigger the cleanup on their NetConnection
- Exposed SpatialOS connection events in `USpatialNetDriver`
- Dynamic Component Ids now start from 10000, Gdk Components will now use 9999 - 0 to avoid future clashes
- Report an error during schema generation if a blueprint RPC has a "by reference" argument
- Launch configs can now be auto-generated to match the selected number of servers to launch from within the PIE editor
- Placeholder entities placed into the generated snapshot are now optional with a UI switch in the SpatialOS Settings
- Implemented updated functionality for UnrealGDKEditorCommandlet: Whenever loading a map for schema/snapshot generation, all sublevels will also be loaded before generation is started
	1. Will now loop through maps (skipping duplicates) during schema generation, to leverage the "iterative schema generation" feature
	2. Accepts an additional argument -MapPaths that can specify a collection of specific maps and/or directories (recursive) containing maps, delimited by semicolons. If not provided, defaults to "All maps in project"
	3. The paths passed in via -MapPaths are flexible

### Bug fixes:
- StartPlayInEditorGameInstance() now correctly call OnStart() on PIE_Client - (@DW-Sebastien)
- Redirect logging in the cloud to output to the correct file
- Changed type of key in `TMap` so Linux build will not give errors
- Disabled loopback of component updates
- Fix hanging on shutdown for PIE when disconnected from SpatialOS
- Fixed an issue which caused a character controller to not be destroyed when leaving the view of an observing client
- Fixed crash on multiserver PIE shutdown
- Fixed single-worker shutdown issues when launching SpatialOS through Unreal Engine 4 with Use - - Single Process unchecked in Play Options
- Fixed crash on closing client from cloud deployment
- Fix `DeleteDynamicEntities` not getting used correctly in shutdown
- Only call `BeginPlay()` on Actors if the World has begun play
- Fixed an issue with schema generation for the default GameMode
- Deleting the schema database reset the starting component ID
- Report invalid name errors during schema generation instead of when launching a deployment.
- `SchemaDatabase` can now be deleted and component ids will reset.
- `COND_InitialOnly` are only replicated once at the start
- Fixed a bug where standalone clients run locally would not connect to spatial
- Missing classes when connecting via a non-editor client
- Schema is now generated for classes that only have RPCs
- Fixed issue where properties won’t be replicated at the start of the game sometimes
- Fixed path bug when specifying snapshot output file in the settings
- Fixed up default connection flows
- Fixed issue will stale shadow data when crossing worker boundaries.
- Removed actors from replication consider list if Unreal server-worker is not authoritative over said actor
- Remove legacy flag "qos_max_unacked_pings_rate" in generated default config - (@DW-Sebastien)

### External contributors:
@DW-Sebastien

## [`0.1.0`](https://github.com/spatialos/UnrealGDK/releases/tag/0.1.0) - 2019-02-08

## Release Notes 0.1.0

Support for the new Player Auth APIs has been added and general stability improvements.

### New Known Issues:
Level streaming is currently not supported.
For other current known issues, please visit [this docs page](https://docs.improbable.io/unreal/alpha/known-issues).

### Features:
* Support for the new Player Auth APIs
* FUniqueNetId support
* Support for the new network protocol KCP
* Lazy loading of FClassInfo
* Augmented BuildWorker.bat to support additional UBT parameters
* Add IsValid() to FUnrealObjRef

### Bug fixes:
* Fixed critical errors related to Unresolved Objects
* Fixed a bug with Player State appearing to be null
* Fixed a bug related to Create Entity responses coming off the wire after a corresponding actor has been deleted
* Fixed a bug with activating actor components. We now check Initial Data for Actor components and only apply updates if `bReplicates` is true
* Fixed a bug when replicating a null list / array
* Fixed a crash with unresolved handover properties
* Changed RakNet to default network protocol temporarily to avoid performance issues with KCP
* Fixed a bug where cloud logging would not work correctly
