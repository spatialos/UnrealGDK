# NUF: Native Unreal Framework

NUF is a proof of concept project that implements Unreal networking over SpatialOS.

##Current scope:
NUF is meant to support only a limited number of classes at this time. Currently, we have basic interoperability for `APlayerController` and `ACharacter` classes. 

##How does NUF work?
At a very high level, NUF does 3 things:
1) Simulate handshake process of an Unreal client and server, using SpatialOS commands
2) Detect changes to replicated properties of replicated actors, and convert them into SpatialOS component updates via generated code.
3) Intercept RPCs, and convert them into SpatialOS commands via generated code.

##Current limitations:
- Link to caveats doc

##How to run:

##Future work:
