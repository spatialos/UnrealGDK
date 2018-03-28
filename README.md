# NUF: Native Unreal Framework

NUF is a proof of concept project that implements Unreal networking over SpatialOS. It is not yet fully functional and is provided here for reference, however we are very open to any feedback regarding the architecture of the project.

NUF is not designed as an SDK, however most of the code here can be pulled into one in the future.

## Current scope:
NUF is meant to support only a limited number of classes at this time. Currently, we have basic interoperability for `APlayerController` and `ACharacter` classes. We also generate interop code for `AGameStateBase` and `APlayerState` to temporarily drive basic connection logic, but this is bound to change.

## How does NUF work?
At a very high level, NUF does 3 things:
1) Simulate handshake process of an Unreal client and server, using SpatialOS commands
2) Detect changes to replicated properties of replicated actors, and convert them into SpatialOS component updates via generated code.
3) Intercept RPCs, and convert them into SpatialOS commands via generated code.

Main components of the project are:

### `GenerateInteropCodeCommandlet`
The code generator will take a set of Unreal classes, and generate routing code (called "type bindings") that enables automated Unreal <-> SpatialOS communication. This file lives in a separate .uproject (`InteropCodeGenerator`) to prevent cross-dependency between generated code and the code generator.

The interop code generator first generates `.schema` files from `UObject` class layouts via Unreals reflection system for SpatialOS to be able to understand and store Unreal data. Then, it generates special `SpatialTypeBinding` classes which are designed to convert property updates to and from SpatialOS (in the form of component updates), and send/receive RPCs via SpatialOS commands. There is rudimentary logic to handle conditional replication based on actor ownership.

The code generator can be run by executing either `generate_code.bat` or `generate_code.sh` after building the `InteropCodeGenerator` project with `Development Editor` at least once.

### `USpatialNetDriver`

Unlike our `UnrealSDK` examples, SpatialOS initialization in NUF happens at the net driver level. `SpatialNetDriver` connects to SpatialOS. Within the `OnSpatialOSConnected` callback, client will send a request to spawn a player.

### `USpatialNetConnection`

How we handle net connections diverges somewhat from Unreal's approach. If a worker is managing N players, NUF will create N + 1 `USpatialNetConnection`'s. The first connection is a special "fall back" connection which is used to write the state of objects managed by the worker to SpatialOS, such as static objects or NPCs. The remaining connections are used by each `PlayerController` managed by the worker, and actors which are owned by each player controller are replicated via their player controllers `USpatialNetConnection`. On the client side, there is just one `USpatialNetConnection`, similar to Unreal's current approach.

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

Note that NUF does not use any other channel type currently. Data that is regularly routed through control channels (e.g. login request) is meant to be implemented directly using SpatialOS commands. See `ASpatialSpawner` for an example.

## Current limitations:
The focus of this project is de-risking SpatialOS interoperability with native code. Some code organization, feature coverage and efficiency trade-offs have been made temporarily to expedite implementation.

Note that a majority of the items below are on our short term roadmap.

- `FName`s are not supported.
- Listen servers haven't been tested extensively. Please use dedicated servers for now.
- For more miscellaneous caveats, see [this](https://docs.google.com/document/d/1dOpA0I2jBNgnxUuFFXtmtu_J1vIPBrlC8r1hAFWHF5I/edit) doc.

## Engine changes:

There is a small number of changes to UE4 source code we have to make. These changes are mostly limited in scope and they only consist of class access, polymorphism, and dll export related changes. We will attempt to consolidate and remove (or submit as PR to Epic) as many of these changes as possible. Our changes can be seen in our `UnrealEngine` repo, `UnrealEngine417_NUF` branch. 

## How to run:

- Build the engine fork, which can be found at `https://github.com/improbable/UnrealEngine`. Make sure to check out the `UnrealEngine417_NUF` branch.
- Set UNREAL_HOME environment variable to the engine fork location.
- Set the uproject to use the engine fork.
- `spatial worker build --target=local`
- `spatial local launch`
- Launch PIE with dedicated server + 1 player.

The interop code and schema generated for marshalling updates and RPCs has been committed directly to the source tree in `workers/unreal/Game/Source/SampleGame/Generated` and `schema/improbable/unreal/generated` respectively. This means you only need to re-run the commandlet if you have changed the code generator.

There are also two non-load-balanced launch scripts to assist your development: 'one_worker_test.json' tests that managed workers launch correctly and ensures that entities never cross worker boundaries. 'two_worker_test.json' provides a static non-overlapping worker boundary between two workers to assist your entity migration testing. As worker boundaries don't overlap, workers have no knowledge of an entity which is under the authority of a different worker.

## Current focus:
We are currently implementing cross-server actor transfers.
