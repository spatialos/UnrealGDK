# What's new in alpha?

If you’ve used the pre-alpha GDK you may be interested in these latest changes.

## Iterative improvements
- Upgraded the engine fork to support Unreal Engine 4.20.3.
- Upgraded to the new SpatialOS load balancer, providing fast multi-worker iteration.
- Removed the incremental build time overhead of using the GDK by implementing dynamic type bindings, avoiding the need to generate certain interop code. You can find more info [here]({{urlRoot}}/content/dynamic-typebindings)
- Schema generation for all classes is now the default.
- Clients no longer automatically connect to a deployment when not in PIE, unless you pass it a commandline argument. For example: 
`"%UNREAL_HOME%\Engine\Binaries\Win64\UE4Editor-Win64-Debug.exe" "%~dp0%PROJECT_PATH%\%GAME_NAME%.uproject" 127.0.0.1`
- Startup actors are now saved into the snapshot.
- Various stability fixes.

## New features
- Authority [is now based on]({{urlRoot}}/content/authority) Spatial authority.
- Provided easy switching between Unreal and SpatialOS networking with a simple checkbox.
- Implemented the GDK as an Unreal plugin, removing the need to symlink UnrealGDK folders.
- Improved component replication support to accommodate attaching multiple components of the same type.
- Facilitated communication with actors for which another server is authoritative, using cross-server RPCs. For more info: <link goes here>
- Implemented `ClientTravel`, allowing clients to be moved between deployments, regardless of whether they are local or cloud.
- Implemented `ServerTravel`, allowing managed servers and external servers (run outside of Unreal Editor) to move between maps and take clients with them.
- Singletons can [now be defined]({{urlRoot}}/content/singleton-actors) as “server only”.
- Initial limited support for Ability System Component on single server only.

