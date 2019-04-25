<%(TOC)%>
# Map travel

This topic is for advanced users only. Before reading this page, make sure you are familiar with the Unreal documentation on [Map travel](https://docs.unrealengine.com/en-us/Gameplay/Networking/Travelling).

## APlayerController::ClientTravel

### In native Unreal
`ClientTravel` is the process of changing which [map (or Level - see Unreal documentation)](http://api.unrealengine.com/INT/Shared/Glossary/index.html#l) a client currently has loaded.

### In the GDK
In the GDK you can use `ClientTravel` to move a client-worker from an offline state to a connected state, where that connection is to a local deployment or a cloud deployment. Alternatively, you can move a connected client-worker to an offline state. You can also change which SpatialOS deployment the client-worker is connected to.  

**Note:** Always make sure your client-worker(s) have the same map loaded as the one which the server-worker(s) are running in your deployment. 

#### Using Receptionist
The Receptionist is a SpatialOS service which allows you to connect to a deployment via a host and port. You can specify these parameters in command line arguments like in native Unreal and you will connect automatically via receptionist.

To connect to a deployment using `ClientTravel` and the receptionist flow, simply call `APlayerController::ClientTravel` with the receptionist IP and port. Make sure to also specify the map that is loaded in the deployment you are connecting to. For example:

```
FString TravelURL = TEXT("127.0.0.1:7777/DestinationMap");
PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute, false /*bSeamless*/);
```
**Note:** The receptionist connection flow is intended to be used for development only. A released game should use the `Locator` flow.

#### Using Locator
The Locator is a SpatialOS service which allows you to connect to cloud deployments. 

> Experimental: You can use `ClientTravel` to change which cloud deployment a client-worker is connected to. This functionality is highly experimental and requires you to write your own authorization code.

Using the locator flow is very similar to using the receptionist, except with different URL options. You must add the `locator`, or `legacylocator` option, and specify the appropriate options.  The `locator` workflow makes use of the new [Authentication flow](https://docs.improbable.io/reference/latest/shared/auth/integrate-authentication-platform-sdk), and the `legacylocator` makes use of the [Deprecated Authentication flow](https://docs.improbable.io/reference/latest/shared/auth/integrate-authentication)

**Locator**: Add the options `locator`, `playeridentity` and `login`.
```
FURL TravelURL;
TravelURL.Host = TEXT("locator.improbable.io");
TravelURL.Map = TEXT("DesiredMap");
TravelURL.AddOption(TEXT("locator"));
TravelURL.AddOption(TEXT("playeridentity=MY_PLAYER_IDENTITY_TOKEN"));
TravelURL.AddOption(TEXT("login=MY_LOGIN_TOKEN"));

PlayerController->ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);
```

**Legacy Locator**: Add `legacylocator`, a project name (the SpatialOS Console project where you have started your deployment), a deployment name (the name you use to start the deployment), and a login token (requires writing authentication code, see [our authentication service docs](https://docs.improbable.io/reference/latest/shared/auth/integrate-authentication) for details).
```
FURL TravelURL;
TravelURL.Host = TEXT("locator.improbable.io");
TravelURL.Map = TEXT("DesiredMap");
TravelURL.AddOption(TEXT("legacylocator"));
TravelURL.AddOption(TEXT("project=MY_PROJECT_NAME"));
TravelURL.AddOption(TEXT("deployment=MY_DEPLOYMENT_NAME"));
TravelURL.AddOption(TEXT("token=MY_LOGIN_TOKEN"));

PlayerController->ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);
```

### ClientTravel - technical details
We have made changes to the Unreal Engine to detect if you have SpatialOS networking enabled. If you do, when you specify a ClientTravel URL containing a host to connect to, we create a `SpatialPendingNetGame` instead of a default Unreal `PendingNetGame`. This internally creates a `SpatialNetConnection` which connects you to the specified host. 

## UWorld::ServerTravel
> Warning: `ServerTravel` is in an experimental state and we currently only support it in single server-worker configurations.   
> We don’t support `ServerTravel` in [PIE](https://docs.unrealengine.com/en-us/GettingStarted/HowTo/PIE#playineditor).

### In native Unreal
`ServerTravel` in Unreal is the concept of changing the [map (or Level - see Unreal documentation)](http://api.unrealengine.com/INT/Shared/Glossary/index.html#l) for the server and all connected clients. A common use case is starting a server in a lobby level. Clients connect to this lobby level and choose loadout and character, for example. When ready, the server triggers a `ServerTravel`, which transitions the deployment and all clients into the main game level.

When `ServerTravel` is triggered, the server tells all clients to begin to [`ClientTravel`](https://docs.unrealengine.com/en-us/Gameplay/Networking/Travelling) to the map specified. If the `ServerTravel` is seamless then the client maintains its connection to the server. If it’s not seamless then all the clients disconnect from the server and reconnect once they have loaded the map. Internally, the server does a similar process: it loads in the new level, usually a game world for all the clients to play on, and begins accepting player spawn requests once ready.

### In the GDK
To use `ServerTravel` with the GDK there are a couple of extra steps to ensure the SpatialOS deployment is in the correct state when transitioning maps. 

#### Generate snapshot 
Generate a snapshot for the map you intend to server transition to using the [snapshot generator]({{urlRoot}}/content/spatialos-concepts/generating-a-snapshot) and save it to `<GameRoot>\Content\Spatial\Snapshots\`. You can either copy your generated snapshot manually (from `<ProjectRoot>\spatial\snapshots\default.snapshot`) or set up your project settings to generate the snapshot for your level into that folder. You can find these settings via **Edit** > **Project Settings** > **SpatialOS Unreal GDK** > **Snapshot path**.

The snapshot is read from `<GameRoot>\Content\Spatial\Snapshots\` when you call the `UWorld::ServerTravel`. To ensure this works in a cloud deployment, add the `Spatial\Snapshots` folder to your  **Additional Non-Asset Directories To Copy for dedicated server only** found at **File** > **Package Project** > **Packaging settings**. For example:

![snapshot asset cooking]({{assetRoot}}assets/screen-grabs/snapshot-asset-cooking.png)

Remember to package the map you intend to travel to: from the **Package Settings**, add your map(s) to **List of maps to include in a packaged build**.

![map cooking]({{assetRoot}}assets/screen-grabs/cooking-maps.png)

#### Specify URL parameters
Pass the snapshot to load as part of the map URL when calling `ServerTravel`. For example:

```
FString ServerTravelURL = TEXT("ExampleMap?snapshot=ExampleMap.snapshot");
UWorld* World = GetWorld();
World->ServerTravel(ServerTravelURL, true /*bAbsolute*/);
```

The GDK has also added the `clientsStayConnected` URL parameter.  
Adding this URL parameter to your `ServerTravel` URL prevents client-workers from disconnecting from SpatialOS during the `ServerTravel` process. We recommend doing this to prevent extra load when client-workers attempt to re-connect to SpatialOS. For example: 

```
FString ServerTravelURL = TEXT("ExampleMap?snapshot=ExampleMap.snapshot?clientsStayConnected");
```

### ServerTravel - technical details
There are a few things to consider when using `ServerTravel` with SpatialOS. It’s not normal for a SpatialOS deployment to ‘load a new world’, since SpatialOS was made for very large persistent worlds. This means you need to perform extra steps to support `ServerTravel`. 

> Note that `ServerTravel` is only supported in non-PIE configurations. Launching a server-worker in PIE has a dependency on the `Use dedicated server` network configuration available in PIE. Unfortunately this setting has a dependency on `Use single process` which doesn’t support `ServerTravel`. To test and develop `ServerTravel` for your game with the SpatialOS GDK for Unreal, you need to launch workers outside of the editor, either via using the the `LaunchServer.bat` or by using a launch configuration which loads [managed workers](https://docs.improbable.io/reference/13.0/shared/concepts/workers#managed-and-external-workers).

The first thing to consider is how to handle SpatialOS’s snapshot system. Normally, you can load a snapshot for a deployment at startup and then forget about it. But since you will be changing worlds, and you generate a single snapshot per world, you need to change the snapshot you have loaded into the deployment at runtime. You can achieve this by ‘wiping’ the deployment: essentially you delete all entities in the world, and when it’s empty you manually load a new snapshot. You specify the snapshot to load in the URL of the `ServerTravel`.

The second thing to consider is how to hande player connections. Normally, once a map is loaded in Unreal on the server, clients can just connect, but for SpatialOS you need to make sure the deployment is in a good state (snapshot fully loaded) before client-workers can connect.

The world wiping is handled at the start of `ServerTravel`, after client-workers have unloaded their current world and started loading a new map, but before server-workers have started unloading their current world. Only the worker which has authority over the [GSM]({{urlRoot}}/content/glossary#global-state-manager) should be able to wipe the world. It uses a large and expensive `EntityQuery` for all entities in the world. Once you have an entity query response for all entities that exist, you send deletion requests for each one. After finishing, the server-workers continue standard Unreal `ServerTravel` and load in the new world.

The snapshot loading is again handled by the server-worker which _was_ authoritative over the GSM (since the GSM entity has now been deleted). The process of loading the snapshot starts once the server-worker with authority has loaded the world (`SpatialNetDriver::OnMapLoaded`). Using the `SnapshotManager`, this server-worker reads the snapshot specified in the map URL from the `Game\Content\Spatial\Snapshots` directory. Iterating through all entities in the snapshot, the worker sends a spawn request for all of them. Once the GSM has been spawned and the spawning process for the rest of the entities has completed, the server-worker which gains authority over the GSM sets the `AcceptingPlayers` field to true. This tells client-workers that they can now send player spawn requests. Client-workers know about the `AcceptingPlayers` state by sending entity queries on a timer for its existence and state.

## Default connection flows
#### In PIE
Launching a [PIE](https://docs.unrealengine.com/en-us/GettingStarted/HowTo/PIE#playineditor) client-worker from the editor will automatically connect it to a local SpatialOS deployment. This is for quick editing and debugging purposes.

#### With built clients
By default, outside of PIE, clients do not connect to a SpatialOS deployment. This is so you can implement your own connection flow, whether that be through an offline login screen, a connected lobby, etc.

To connect a client-worker to a deployment from an offline state, you must use [`ClientTravel`](#aplayercontroller-clienttravel).

The `LaunchClient.bat` (which we have provided) already includes the local host IP `127.0.0.1` which means client-workers launched this way will attempt to connect automatically using the [receptionist](#using-receptionist) flow.  


#### With the Launcher
When launching a client-worker from the SpatialOS Console using the [Launcher](https://docs.improbable.io/reference/latest/shared/operate/launcher#the-launcher), the client-worker will connect to SpatialOS by default. It has the `Locator` information required to connect to said deployment included as command-line arguments. When these `Locator` arguments are present, client-workers will attempt to connect automatically. Please note the launcher login tokens are only valid for 15 minutes.  

> Connecting by default when using the launcher is subject to change.
