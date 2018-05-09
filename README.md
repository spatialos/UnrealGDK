# Spatial GDK: Unreal Module for Spatial OS development

## TOC

* [Getting started](#getting-started)
    * [Pre-requisites](#pre-requisites)
    * [Building the GDK and installing it to the SampleGame](#building-the-gdk-and-installing-it-to-the-samplegame)
        * [For End-Users](#for-end-users)
        * [For Local development](#for-local-development)
* [How does the Spatial GDK work?](#how-does-the-spatial-gdk-work?)
    * [GenerateInteropCodeCommandlet](#generateinteropcodecommandlet)
    * [USpatialNetDriver](#uspatialnetdriver)
    * [USpatialNetConnection](#uspatialnetconnection)
    * [USpatialInterop](#uspatialinterop)
    * [USpatialInteropPipelineBlock](#uspatialinteroppipelineblock)
    * [USpatialPackageMapClient](#uspatialpackagemapclient)
    * [USpatialActorChannel](#uspatialactorchannel)
* [Folder layout](#folder-layout)
* [Engine changes](#engine-changes)
* [Known issues](#known-issues)


------
## Getting started

### Pre-requisites

To build the Unreal GDK you will need to have the following items installed on your machine:

* Bash
* Improbable toolshare
* A built out fork of our Unreal GDK. (4.19) (See [Engine changes](#engine-changes))

**Note** It is currently only possible to build a the GDK if you are an Improbable engineer due to the dependency on codegen. This will be removed, either when we completely remove the dependency on the SDK or if we fork [imp_lint](https://github.com/improbable/platform/tree/master/go/src/improbable.io/cmd/imp_lint) and [imp_nugget](https://github.com/improbable/platform/tree/master/go/src/improbable.io/cmd/imp-nuget) from the platform repo.

### Building the GDK and installing it to the SampleGame

The following steps are required to get the GDK built and installed to the SampleGame:

#### For End-Users

1. Clone the [SampleGame repo](https://github.com/improbable/unreal-gdk-sample-game/)
2. Run the script `setup.sh <path to your SampleGame root foder>`. This will build the GDK and copy the `Binaries`, `Script`, `SpatialGDK` and `Plugins/SpatialGDK` folders from the GDK repo to the right location in your SampleGame repo.
3. Build the SampleGame using `spatial build --target=local`

### For Local development

1. Clone the [GDK repo](https://github.com/improbable/unreal-gdk).
2. Run the script `ci/build.sh`
3. Run the script `create_spatialGDK_symlink.bat` within the SampleGame repo.

------

## How does the Spatial GDK work?
At a very high level, Spatial GDK does 3 things:
1) Simulate the handshake process of an Unreal client and server, using SpatialOS commands
2) Detect changes to replicated properties of replicated actors, and convert them into SpatialOS component updates via generated code.
3) Intercept RPCs, and convert them into SpatialOS commands via generated code.

Main components of the project are:

### `GenerateInteropCodeCommandlet`
The code generator will take a set of Unreal classes, and generate routing code (called "type bindings") that enables automated Unreal <-> SpatialOS communication. This file lives in a separate .uproject (`InteropCodeGenerator`) to prevent cross-dependency between generated code and the code generator.

First, the interop code generator generates `.schema` files from `UObject` class layouts via Unreal's reflection system. This enables SpatialOS to understand and store Unreal data. Then, it generates `SpatialTypeBinding` classes. These classes:
* Convert property updates to and from SpatialOS in the form of component updates.
* Send and receive RPCs via SpatialOS commands.
* Have rudimentary logic to handle conditional replication based on actor ownership.

To run the code generator, make sure you have built the `InteropCodeGenerator` project with `Development Editor` at least once. Then execute either `generate_code.bat` or `generate_code.sh`

### `USpatialNetDriver`

Unlike our `UnrealSDK` examples, SpatialOS initialisation in Spatial GDK happens at the net driver level. `SpatialNetDriver` connects to SpatialOS. Within the `OnSpatialOSConnected` callback, the client sends a request to spawn a player.

### `USpatialNetConnection`

How we handle net connections diverges somewhat from Unreal's approach. If a worker is managing N players, Spatial GDK creates N + 1 `USpatialNetConnection`s. The first connection is a special "fall back" connection; the GDK uses this connection to write to SpatialOS the state of objects managed by the worker. These objects could be NPCs or static objects, for example. The remaining connections are used by each `PlayerController` managed by the worker. Actors owned by each `PlayerController` are replicated via their player controller's `USpatialNetConnection`. On the client side, there is just one `USpatialNetConnection`, similar to Unreal's current approach.

### `USpatialInterop`

This layer is responsible for most of the conversion between Unreal property updates and SpatialOS component updates, plus RPCs. When there is a change, it invokes serialization code through the auto-generated type bindings (one for each supported actor class, discussed above) and packs the changes into Spatial updates.

Moreover, RPC invocations are intercepted by this layer and directed through appropriate class bindings where they turn into SpatialOS commands.

Note that this class is responsible for dealing with the asynchronous nature of SpatialOS entity spawning. For example, it might not be possible to resolve an object property update or RPC parameter immediately if the entity that corresponds to the relevant actor has not been created by SpatialOS yet. There are mechanisms built into this layer that retry sending these operations upon resolving actor <-> entity associations.

### `USpatialInteropPipelineBlock`

This file extends on the entity pipeline block concept that is present in our UnrealSDK. The main functionality here is maintaining a registry of `AActor` <-> SpatialOS entity associations. There are multiple scenarios for these associations, depending on who the original worker that spawns the actor is, the type of worker, and the type of actor.

### `USpatialPackageMapClient`

Transmitting `UObject` references over the wire requires extra care. We encode each UObject reference in SpatialOS terms in a structure called `UnrealObjectRef`, and the workers translate these to pointers to their local objects using a GUID system that builds on Unreal's implementation.

### `USpatialActorChannel`:

This class is in charge of triggering initial serialization and SpatialOS entity creation in response to `SpawnActor()` calls, and per tick delta detection in replicated actor properties. There is one per replicated actor in our implementation, as opposed to one per replicated actor per connected player.

Note that Spatial GDK does not use any other channel type currently. Data that is regularly routed through control channels (e.g. login request) is meant to be implemented directly using SpatialOS commands. See `ASpatialSpawner` for an example.

## Folder layout

| Directory | Purpose
|-----------|---------
| `Binaries/ThirdParty/Improbable/` | (Not tracked in git) This folder contains all binaries required for building a gdk project. These are built usin `ci/build.sh`
| `build/` | (Not tracked in git) Intermediate folder used for temporary steps in the build process of the GDK.
| `build_scripts/` | Contains the templates for the default worker builds scripts required to build a GDK worker.
| `ci/` | Contains the CI build scripts required to build the GDK
| `ci/linting/` | Contains the linting scripts required to build the GDK
| `go/unreal_packager` | A stand-alone application that is used to trigger the build and packaging of GDK workers
| `go/cleanup/` | A stand-alone application used to clean GDK workers.
| `packages/` | (Not tracked in git) Contains the dependencies for building the old SDK codegen.
| `Plugins/SpatialGDK/SpatialGDKEditorToolbar/` | The GDK editor toolbar containing the interop codegen
| `Plugins/SpatialGDK/SpatialOSEditorToolbar/` | Contains the source of the forked [unified Unreal SDK SpatialOS editor toolbar](https://github.com/improbable/unified-unreal-sdk/tree/master/Plugins/SpatialOS/SpatialOSEditorToolbar/Source/SpatialOSEditorToolbar).
| `Scripts/` | Contains the  the processed worker build scripts used to build a GDK worker.
| `Source/Programs/` | Contains the forked version of the [unified Unreal SDK code generator](https://github.com/improbable/unified-unreal-sdk/tree/master/Source/Programs/Improbable.Unreal.CodeGeneration).
| `Source/SpatialGDK/Legacy/` | Contains the forked source code from the [unified Unreal SDK](https://github.com/improbable/unified-unreal-sdk/tree/master/Source/SpatialOS).
| `Source/SpatialGDK/Legacy/Deprecated/` | Contains the forked source code from the [unified Unreal SDK](https://github.com/improbable/unified-unreal-sdk/tree/master/Source/SpatialOS) that are not dependencies for the GDK and purely present for backwards compatibility.
| `Source/SpatialGDK/Public/WorkerSdk/` | (Not tracked in git) The worker sdk headers. These are installed when running `ci/build.sh`
| `Source/SpatialGDK/Public` | Public source code of the Unreal GDK module
| `Source/SpatialGDK/Private` | Private source code of the Unreal GDK module

## Engine changes

There is a small number of changes to UE4 source code we have to make. These changes are mostly limited in scope and they only consist of class access, polymorphism, and dll export related changes. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. Our changes can be seen in our `UnrealEngine` repo, `UnrealEngine419_SpatialGDK` branch.

## Known issues
The focus of this project is de-risking SpatialOS interoperability with native code. Some code organization, feature coverage and efficiency trade-offs have been made temporarily to expedite implementation.

Note that a majority of the items below are on our short term roadmap.

- `FName`s are not supported.
- Listen servers haven't been tested extensively. Please use dedicated servers for now.
- For more miscellaneous caveats, see [this](https://docs.google.com/document/d/1dOpA0I2jBNgnxUuFFXtmtu_J1vIPBrlC8r1hAFWHF5I/edit) doc.
