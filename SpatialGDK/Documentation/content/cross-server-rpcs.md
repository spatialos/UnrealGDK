<%(TOC)%>
# Cross-server RPCs

In native-Unreal networking, [RPCs (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs) are functions which either the client or the server use to send messages to each other over a network connection. 

Cross-server RPCs facilitate [zoning]({{urlRoot}}/content/glossary#zoning), which is one of the GDK's options for multiserver development.

> **Note:** Support for zoning is currently in pre-alpha. We invite you to try out the [multiserver zoning tutorial]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro) and learn about how it works, but we don’t recommend you start developing features that use zoning yet.

In Unreal’s native single-server architecture, your game server holds the canonical state of the whole game world. As there is a single game server, it has complete authority over all server Actors and so it is able to invoke and execute functions on Actors unhindered. 

In SpatialOS games, there can be more than one server; these multiple servers are known as “server-workers”. (Find out more about server-workers as well as “client-workers” in the [glossary]({{urlRoot}}/content/glossary#worker).) In a SpatialOS game that runs across many server-workers, SpatialOS server-workers have the concept of “worker authority” - where only one server-worker at a time is able to invoke and execute functions on Actors. (Find out more about authority in the [glossary]({{urlRoot}}/content/glossary#authority).)

As Unreal expects there to be only one server, rather than several servers, the GDK has a custom solution to take advantage of the SpatialOS distributed server architecture. This involves handling the scenario where a server-worker attempts to invoke an RPC on an Actor that another server-worker has [authority]({{urlRoot}}/content/glossary#worker) over. This custom solution is the cross-server RPC. The GDK offers cross-server RPC in addition to support for the [native RPC types that Unreal provides (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs).

When a cross-server RPC is invoked by a server-worker, SpatialOS routes the execution through the SpatialOS [Runtime]({{urlRoot}}/content/glossary#spatialos-runtime) to the server-worker that has authority - this server-worker executes the RPC.

The example diagram below shows a player successfully shooting another player’s hat across a server-worker boundary.

![A situation where you might need cross-server RPCs]({{assetRoot}}assets/shooting-workflow-simple.png)

In the diagram, Server-worker 1 has authority over Player 1 and Server-worker 2 has authority over Player 2. If Player 1 shoots a bullet, Server-worker 1 knows about the bullet and can make any necessary changes to Player 1 but it can’t make changes to Player 2 when the bullet hits. SpatialOS ensures that Server-worker 2 can make changes to Player 2 (the hat gets hit by the bullet) by routing the change notification from Server-worker 1 to Server-worker 2.

### How to send a cross-server RPC (using C++)

To set up a cross-server RPC, follow the same instructions as you would for [marking up RPCs (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs) within Unreal.

1. Add a `CrossServer` tag to the `UFUNCTION` macro of the RPC function that you want to be cross-server on your Actor (`MyActor` in this example).

    ```
    UFUNCTION(CrossServer, Reliable, WithValidation)
    void MyCrossServerRPC();
    ```

    Note: `WithValidation` is optional.

1. Add the related function implementations:
    ```
    void MyActor::MyCrossServerRPC_Implementation()
    {
        // Implementation goes here...
    }
   ```
   Note: You may need to implement the `MyCrossServerRPC_Validation()` if you used the `WithValidation` attribute.

1. Invoke the `CrossServer` RPC function as you would with any other function.

### How to send a cross-server RPC (using Blueprints)

To set up a cross-server RPC in a Blueprint, follow the same instructions as you would for [marking up RPCs in Blueprints (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs#blueprints), but from the **Replicates** drop-down list within the **Details** panel of your event, select **Run on authoritative server (sent from server)**:

![Setting up a cross-server RPC in blueprint]({{assetRoot}}assets/screen-grabs/crossserver-blueprint.png)

### Execution notes

The tables below show where cross-server RPCs are executed based on where they were invoked. (To make it easier to follow, the tables use the same format as the [Unreal documentation on RPCs](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs#rpcinvokedfromtheserver).)

#### Invoking a cross-server RPC from a server-worker that has authority over an Actor

| **Actor ownership** | **Cross-server RPC**
|-----------|---------
| Client-owned Actor | Runs on the server-worker that has authority
| Server-owned Actor | Runs on the server-worker that has authority
| Unowned Actor | Runs on the server-worker that has authority

#### Invoking a cross-server RPC from a client-worker

The call is not processed because this type of RPC is only for a server-worker instance to call.

### Unresolved parameters in an RPC

In some situations, a worker (either a client or a server) may receive an RPC with a parameter that it cannot resolve. In these instances, the GDK will output a warning. However, sometimes this behaviour is acceptable. To account for these cases where the behaviour is acceptable, you can disable the warnings on a per-RPC basis, either using C++ or using Blueprints.

#### How to disable warnings for unresolved parameters in an RPC (using C++)

To disable these warnings on an RPC in C++, add an `AllowUnresolvedParameters` tag to the `UFUNCTION` macro of the RPC function.

```
UFUNCTION(CrossServer, AllowUnresolvedParameters, Reliable, WithValidation)
void MyCrossServerRPC();
```

#### How to disable warnings for unresolved parameters in an RPC (using Blueprints)

To disable these warnings on an RPC in a Blueprint, in the **Details** panel of the event, click the eye in the top-right, and make sure **Show All Advanced Details** is ticked. The **Allow Unresolved Parameters** field in the panel will appear.

<br/>------<br/>
_2019-06-06 Page updated with limited editorial review_
<br/>
_2019-06-06 Updated invoking a cross-server RPC from a client worker guidance_
