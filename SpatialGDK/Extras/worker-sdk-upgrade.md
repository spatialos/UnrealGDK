# Upgrading the Worker SDK version

When upgrading the Worker SDK version for the Unreal GDK, you also need to follow some additional steps to ensure that the Worker SDK works with Unreal.

## Update the OpenSSL version for iOS

Check whether the Worker SDK has changed their OpenSSL version. If yes, check in the [OpenSSL.Build.cs](https://github.com/improbableio/UnrealEngine/blob/4.22-SpatialOSUnrealGDK-release/Engine/Source/ThirdParty/OpenSSL/OpenSSL.Build.cs) whether we link compatible headers for iOS. If not, you will have to update this file and add the necessary headers to the Engine.
This is necessary to ensure that iOS works as expected. 

To test whether it works you need to start a local deployment, build out an iOS client that is able to connect to that local deployment and see whether it crashes. If the versions are not compatible, it should fail when calling `ConnectAsync` inside the Worker SDK due to the wrong symbols being loaded. 