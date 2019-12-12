
# Map travel

This topic is for advanced users only. Before reading this page, make sure you are familiar with the Unreal documentation on [Map travel](https://docs.unrealengine.com/en-us/Gameplay/Networking/Travelling).

## APlayerController::ClientTravel

### In native Unreal
`ClientTravel` is the process of changing which [map (or Level - see Unreal documentation)](http://api.unrealengine.com/INT/Shared/Glossary/index.html#l) a client currently has loaded. It is also used to connect to a server or change servers.

### In the GDK
In the GDK you can use `ClientTravel` to connect to a SpatialOS deployment, where that connection is to a local deployment or a cloud deployment. You can also change the current SpatialOS deployment the client is connected to.

By specifying a URL as the first command line parameter when launching a client you can automatically connect clients to a deployment. There are two options available in SpatialOS, the [receptionist](#using-receptionist-local-deployments) flow for local deployments and the [locator](#using-locator-cloud-deployments) flow for cloud deployments. Alternatively you can connect from an offline client to a deployment in-game by using APlayerController::ClientTravel ([OpenLevel](https://api.unrealengine.com/INT/BlueprintAPI/Game/OpenLevel/index.html) in blueprints).

Please refer to the [Map Travel URL options]({{urlRoot}}/content/command-line-arguments#map-travel-url-options) section for all possible SpatialOS URL options.

#### Using Receptionist - Local Deployments
The Receptionist is a SpatialOS service which allows you to connect to a deployment using the host IP and port of the SpatialOS deployment. You can specify these parameters in command line arguments when booting a client and you will connect automatically via receptionist. An example of the receptionist being used to automatically connect a client on boot can be found in the `LaunchClient.bat` script found in our [UnrealGDKExampleProject](https://github.com/spatialos/UnrealGDKExampleProject).

To connect to a deployment using `ClientTravel` and the receptionist flow, simply call `APlayerController::ClientTravel` with the receptionist IP of the host machine and the port the SpatialOS deployment is running on. For example:

[block:code]
{
  "codes": [
  {
      "code": "FString TravelURL = TEXT(\"127.0.0.1:7777\"); \n PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute, false /*bSeamless*/);",
      "language": "text"
    }
  ]
}
[/block]

#### Using Locator - Cloud Deployments
The Locator is a SpatialOS service which allows you to connect to cloud deployments. 

> Experimental: You can use `ClientTravel` to change which cloud deployment a client is connected to. This functionality is experimental and requires you to write your own authorization code.

Using the locator flow is very similar to using the receptionist, except with different URL options. You must add the `locator` option, and specify the appropriate options.  The `locator` workflow makes use of the new [Authentication flow](https://docs.improbable.io/reference/latest/shared/auth/integrate-authentication-platform-sdk).

**Locator**: Add the options `locator`, `playeridentity` and `login`. For more information about the options, see [Map Travel URL options]({{urlRoot}}/content/command-line-arguments#map-travel-url-options).
[block:code]
{
  "codes": [
  {
      "code": " FURL TravelURL; \n TravelURL.Host = TEXT(\"locator.improbable.io\"); \n TravelURL.AddOption(TEXT(\"locator\")); \n TravelURL.AddOption(TEXT(\"playeridentity=MY_PLAYER_IDENTITY_TOKEN\")); \n TravelURL.AddOption(TEXT(\"login=MY_LOGIN_TOKEN\")); \n PlayerController->ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false /*bSeamless*/);",
      "language": "text"
    }
  ]
}
[/block]

## UWorld::ServerTravel
> Warning: `ServerTravel` is on the [roadmap](https://github.com/spatialos/UnrealGDK/projects/1#card-22461878) but is not currently supported.

## Default connection flows
#### Play In Editor Clients
Launching a [PIE](https://docs.unrealengine.com/en-us/GettingStarted/HowTo/PIE#playineditor) client from the editor will automatically attempt to connect to a local SpatialOS deployment on your machine by using the receptionist flow. It uses the URL `127.0.0.1` This is for quick editing and debugging purposes.

By default in the editor and when [Spatial Networking]({{urlRoot}}/content/unreal-editor-interface/toolbars#switching-between-native-unreal-networking-and-spatialos-networking) is enabled clients and servers will automatically wait for a local SpatialOS deployment to be started by the editor before attempting to connect. If you wish to disable this functionality (e.g. if you want to start a deployment outside of the editor) you can change the "Auto-start local deployment" option in the [SpatialOS Editor Settings]({{urlRoot}}/content/unreal-editor-interface/editor-settings).

#### With built clients
By default, outside of PIE, clients do not connect to a SpatialOS deployment unless specific URL parameters are included as command-line arguments, not specifying a URL will result in an offline client. This is so you can implement your own connection flow, whether that be through an offline login screen, a connected lobby, etc. You can specify URL parameters at the command-line the same way as native Unreal with the first argument being the URL to use to travel to.

To connect a client to a deployment from an offline state, you must use [`ClientTravel`](#aplayercontroller-clienttravel).

The `LaunchClient.bat` helper script (which we have provided with the UnrealGDKExampleProject) already includes the local host IP `127.0.0.1` which means client launched this way will attempt to connect automatically using the [receptionist](#using-receptionist-local-deployments) flow.

#### With the SpatialOS Launcher
When launching a client from the SpatialOS Console using the [Launcher](https://docs.improbable.io/reference/latest/shared/operate/launcher#the-launcher), the client will connect to the running SpatialOS cloud deployment by default. It has the `Locator` parameters required to connect to said deployment included as command-line arguments. When these `Locator` arguments are present, client will attempt to connect automatically. Please note the launcher login tokens are only valid for 15 minutes.

## URL Options
Please see the [Map Travel URL options]({{urlRoot}}/content/command-line-arguments#map-travel-url-options) section for a list of all URL options that can be used with SpatialOS.

<br/>------<br/>
_2019-07-31 Page updated with limited editorial review: Updated to match current connection flows. Removed ServerTravel_  
_2019-06-13 Page updated with limited editorial review: Added Locator information_
