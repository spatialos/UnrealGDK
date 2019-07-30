<%(TOC)%>
# URLs and Map travel

This topic is for advanced users only. Before reading this page, make sure you are familiar with the Unreal documentation on [Map travel](https://docs.unrealengine.com/en-us/Gameplay/Networking/Travelling).

## APlayerController::ClientTravel

### In native Unreal
`ClientTravel` is the process of changing which [map (or Level - see Unreal documentation)](http://api.unrealengine.com/INT/Shared/Glossary/index.html#l) a client currently has loaded. It is also used to connect to a server or change servers.

### In the GDK
In the GDK you can use `ClientTravel` to connect to a SpatialOS deployment, where that connection is to a local deployment or a cloud deployment. You can also change the current SpatialOS deployment the client is connected to.

By specifying a URL (TODO: Link to Unreal) as the first command line parameter when launching a client you can automatically connect clients to a deployment. There are two options available in SpatialOS, the Receptionist flow for local deployments and the Locator flow for cloud deployments. Alternatively you can connect from an offline state to a deployment in-game by using APlayerController::ClientTravel.

Please refer to the URL Parameters Glossary (TODO: Link) for all possible SpatialOS URL options.

**Note:** Always make sure your client(s) have the same map loaded as the one which the server(s) are running in your deployment. 

#### Using Receptionist - Local Deployments
The Receptionist is a SpatialOS service which allows you to connect to a deployment via a host and port. You can specify these parameters in command line arguments like in native Unreal and you will connect automatically via receptionist. An example of the receptionist being used to automatically connect a client on boot can be found in the `LaunchClient.bat` script found in our `UnrealGDKExampleProject`(TODO: Link)

To connect to a deployment using `ClientTravel` and the receptionist flow, simply call `APlayerController::ClientTravel` with the receptionist IP and port. For example:

```
FString TravelURL = TEXT("127.0.0.1:7777");
PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute, false /*bSeamless*/);
```

#### Using Locator - Cloud Deployments
The Locator is a SpatialOS service which allows you to connect to cloud deployments. 

> Experimental: You can use `ClientTravel` to change which cloud deployment a client-worker is connected to. This functionality is experimental and requires you to write your own authorization code.

Using the locator flow is very similar to using the receptionist, except with different URL options. You must add the `locator`, or `legacylocator` option, and specify the appropriate options.  The `locator` workflow makes use of the new [Authentication flow](https://docs.improbable.io/reference/latest/shared/auth/integrate-authentication-platform-sdk), and the `legacylocator` makes use of the [Deprecated Authentication flow](https://docs.improbable.io/reference/latest/shared/auth/integrate-authentication)

**Locator**: Add the options `locator`, `playeridentity` and `login`. For more information about the options, see [Map travel URL options]({{urlRoot}}/content/command-line-arguments#map-travel-url-options).
```
FURL TravelURL;
TravelURL.Host = TEXT("locator.improbable.io");
TravelURL.AddOption(TEXT("locator"));
TravelURL.AddOption(TEXT("playeridentity=MY_PLAYER_IDENTITY_TOKEN"));
TravelURL.AddOption(TEXT("login=MY_LOGIN_TOKEN"));

PlayerController->ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);
```

**Legacy Locator**: Add `legacylocator`, a project name (the SpatialOS Console project where you have started your deployment), a deployment name (the name you use to start the deployment), and a login token (requires writing authentication code, see [our authentication service docs](https://docs.improbable.io/reference/latest/shared/auth/integrate-authentication) for details).
```
FURL TravelURL;
TravelURL.Host = TEXT("locator.improbable.io");
TravelURL.AddOption(TEXT("legacylocator"));
TravelURL.AddOption(TEXT("project=MY_PROJECT_NAME"));
TravelURL.AddOption(TEXT("deployment=MY_DEPLOYMENT_NAME"));
TravelURL.AddOption(TEXT("token=MY_LOGIN_TOKEN"));

PlayerController->ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);
```

### ClientTravel - technical details
We have made changes to the Unreal Engine to detect if you have SpatialOS networking enabled. If you do, when you specify a ClientTravel URL containing a host to connect to, we create a `SpatialPendingNetGame` instead of a default Unreal `PendingNetGame`. This internally creates a `SpatialNetConnection` which connects you to the specified host. 

## UWorld::ServerTravel
> Warning: `ServerTravel` is on the [roadmap](https://github.com/spatialos/UnrealGDK/projects/1#card-22461878) but is not currently supported.

## Default connection flows
#### Play In Editor Clients
Launching a [PIE](https://docs.unrealengine.com/en-us/GettingStarted/HowTo/PIE#playineditor) client from the editor will automatically attempt to connect to a local SpatialOS deployment on your machine by using the receptionist flow. It uses the URL `127.0.0.1` This is for quick editing and debugging purposes.

By default in the editor and when Spatial Networking is enabled (TODO: Link) clients and servers will automatically wait for a local SpatialOS deployment to be started by the editor before attempting to connect. If you wish to disable this functionality (e.g. if you want to start a deployment outside of the editor) you can change the "Automatically connect to local deployment" option in the "SpatialOS Editor Settings".

#### With built clients
By default, outside of PIE, clients do not connect to a SpatialOS deployment unless specific URL parameters are included as command-line arguments, not specifying a URL will result in an offline client. This is so you can implement your own connection flow, whether that be through an offline login screen, a connected lobby, etc. You can specify URL parameters at the command-line the same way as native Unreal with the first argument being the URL to use to travel to.

To connect a client to a deployment from an offline state, you must use [`ClientTravel`](#aplayercontroller-clienttravel).

The `LaunchClient.bat` (which we have provided with the UnrealGDKExampleProject) already includes the local host IP `127.0.0.1` which means client launched this way will attempt to connect automatically using the [receptionist](#using-receptionist) flow.

#### With the SpatialOS Launcher
When launching a client from the SpatialOS Console using the [Launcher](https://docs.improbable.io/reference/latest/shared/operate/launcher#the-launcher), the client-worker will connect to SpatialOS by default. It has a `Locator` URL required to connect to said deployment included as command-line arguments. When these `Locator` arguments are present, client will attempt to connect automatically. Please note the launcher login tokens are only valid for 15 minutes.  

> Connecting by default when using the launcher is subject to change.

## URL Parameters


<br/>------<br/>
_2019-06-13 Page updated with limited editorial review: Added Locator information_