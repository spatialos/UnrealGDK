# What's new in alpha?

If you’ve used the GDK in pre-alpha, you may be interested in the latest changes.

## Iterative improvements
- Upgraded the SpatialOS Unreal engine fork to support Unreal Engine 4.20.3.
- Upgraded to the new SpatialOS load balancer, providing fast multi-[worker]({{urlRoot}}/content/glossary#workers) iteration.
- Removed the incremental build-time overhead of using the GDK by implementing [Dynamic Typebindings]({{urlRoot}}/content/glossary#dynamic-typebindings). This removes the additional code generation and compile step.
- [Schema]({{urlRoot}}/content/glossary#dynamic-typebindings) generation is now the default for all classes.
- Clients no longer automatically connect to a [deployment]({{urlRoot}}/content/glossary#deployment) when not in PIE, unless you pass it a command-line argument. <br/>
For example: 

    ```
    "%UNREAL_HOME%\Engine\Binaries\Win64\UE4Editor-Win64-Debug.exe" "%~dp0%PROJECT_PATH%\%GAME_NAME%.uproject" 127.0.0.1
    ```
- Various stability fixes.

## New features
- [Authority]({{urlRoot}}/content/authority) is now based on SpatialOS authority.
- There is easy switching between native-Unreal networking and SpatialOS networking with a [toolbar checkbox]({{urlRoot}}/content/toolbars#switching-between-native-unreal-networking-and-spatialos-networking).
- The GDK is an Unreal plugin, so you don't need to symlink `UnrealGDK` directories.
- Improved [component]({{urlRoot}}/content/glossary#spatialos-component) replication support allows you to attach multiple components of the same type to an Actor (or "[entity]({{urlRoot}}/content/glossary#spatialos-entity)" in SpatialOS).
- [Cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs) facilitate communication with Actors for which another server is [authoritative]({{urlRoot}}/content/authority).
- `ClientTravel` allows you to move clients between [deployments]({{urlRoot}}/content/glossary#deployment), regardless of whether these deployments are local or in the cloud. (See the documentation on [Map travel]({{urlRoot}}/content/map-travel) for further information.)
- `ServerTravel` allows server-workers in [local or cloud deployments]({{urlRoot}}/content/glossary#deployment) to move between maps and take clients with them. (See the documentation on [Map travel]({{urlRoot}}/content/map-travel) for further information.)
- You can define [Singleton Actors]({{urlRoot}}/content/singleton-actors) as “server only”.
- There is initial limited support for Unreal's Ability System Component. This is available on single-server implementations only.
