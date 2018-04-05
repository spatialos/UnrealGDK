# Spatial GDK: Unreal Module for Spatial OS development

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

## Current limitations:
The focus of this project is de-risking SpatialOS interoperability with native code. Some code organization, feature coverage and efficiency trade-offs have been made temporarily to expedite implementation.

Note that a majority of the items below are on our short term roadmap.

- `FName`s are not supported.
- Listen servers haven't been tested extensively. Please use dedicated servers for now.
- For more miscellaneous caveats, see [this](https://docs.google.com/document/d/1dOpA0I2jBNgnxUuFFXtmtu_J1vIPBrlC8r1hAFWHF5I/edit) doc.

## Engine changes:

There is a small number of changes to UE4 source code we have to make. These changes are mostly limited in scope and they only consist of class access, polymorphism, and dll export related changes. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. Our changes can be seen in our `UnrealEngine` repo, `UnrealEngine417_NUF` branch. 

## How to use:

### SpatialGDK module
To register the module with your Unreal project, copy the `SpatialGDK` folder into your project's `Source` folder.

### SpatialGDK editor plugin
To register the editor plugin with your Unreal project, copy the `Plugins\SpatialGDK` into your project's `Plugins` folder.

### InteropCodeGenerator
TODO: revisit when we have a user-definied workflow