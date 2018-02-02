(Readme is WIP currently)

# NUF: Native Unreal Framework

NUF is a proof of concept project that implements Unreal networking over SpatialOS.

The focus is de-risking this functionality. Some code organization and efficiency trade-offs have been made temporarily to expedite implementation.

NUF is not designed as an SDK, however most of the code here can be pulled into one in the future.


##Current scope:
NUF is meant to support only a limited number of classes at this time. Currently, we have basic interoperability for `APlayerController` and `ACharacter` classes. We also generate interop code for `GameState` and `PlayerState` to temporarily drive basic connection logic, but this is bound to change.


##How does NUF work?
At a very high level, NUF does 3 things:
1) Simulate handshake process of an Unreal client and server, using SpatialOS commands
2) Detect changes to replicated properties of replicated actors, and convert them into SpatialOS component updates via generated code.
3) Intercept RPCs, and convert them into SpatialOS commands via generated code.


##Current limitations:
- Re-connecting to an existing deployment is quite buggy. We recommend restarting the deployment before reconnecting for the time being.
- We only support connecting through PIE instances.
- Link to caveats doc


##How to run:


##Future work:
- Move away from using `FArchive`s when converting a Spatial update to Unreal.