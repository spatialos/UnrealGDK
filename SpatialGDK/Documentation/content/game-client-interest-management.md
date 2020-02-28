<%(TOC)%>
# Game client interest management

The GDK uses [interest]({{urlRoot}}/content/glossary#interest) to control the information that [game clients]({{urlRoot}}/content/glossary#client-workers) receive about the world around them.

Game clients need information about Actors that they don’t own (see the Unreal documentation on [owning connections](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/OwningConnections)) to help them to correctly manipulate the Actors that they _do_ own, and to render the SpatialOS world. Interest enables game clients to receive the relevant information.

You can define your game client’s interest in three ways, which you can use alongside each other:

* **`NetCullDistanceSquared`**<br>
A property defined on an Actor's class. For any instance of an Actor of that class or a derived class, game clients whose players are within the distance specified receive updates about the Actor.<br>
* **`ActorInterestComponent`**<br>
An Unreal Actor component that you add to an Actor that a game client might own. This includes the game client's PlayerController, the Pawn that the PlayerController is possessing, and any Actor that the possessed Pawn owns.

    Within this component, you provide a list of queries that express the Actors that you want the game client to receive updates about. This is a more granular way of setting up interest than `NetCullDistanceSquared`.<br>
* **`AlwaysInterested`**<br>
A `UPROPERTY` that you add to an Actor that a game client might own. It refers to a specific Actor that you want the game client to always receive updates about.

> **Tip**: All three ways of defining interest can coexist. However, if you've specified all of the interest in your game using `ActorInterestComponent` and `AlwaysInterested`, you can turn off `NetCullDistanceSquared` using the toggle in `ActorInterestComponent`.

## NetCullDistanceSquared
`NetCullDistanceSquared` is a property on an Actor class that defines a distance. Game clients whose players are within this distance of an Actor of this class receive updates about the Actor. By default, this distance is 150 meters for all Actor classes.

To change an Actor class’s `NetCullDistanceSquared` value, open the Actor class's Blueprint and go to the Class Defaults tab. Net Cull Distance Squared is in the Replication section:

![NetCullDistanceSquared in the Replication section]({{assetRoot}}assets/screen-grabs/game-client-interest-management/replication-section.png)
<br>_Image: Net Cull Distance Squared is in the Replication section of the Class Defaults tab_

> **Tip**: You need to convert your chosen number of meters into centimeters, square it, and then enter the resulting value. For example, to set a distance of 150 meters, you need to enter 225000000 (like in the screenshot above).

### Example 
In your game you have an Actor Blueprint called `PlayerCharacter`, which represents players within your game. If you set `PlayerCharacter`'s `NetCullDistanceSquared` to 200 meters, then all game clients will receive updates about all `PlayerCharacter`s that are within 200 meters of their own player.

If you also have an Actor Blueprint called `Vehicle`, and you set its `NetCullDistanceSquared` to 400 meters, then game clients will receive updates about all `Vehicle`s within 400 meters of their player.

![NetCullDistanceSquared diagram]({{assetRoot}}assets/screen-grabs/game-client-interest-management/net-cull-distance-diagram.png)
<br>_Image: A game client receives updates about Characters and vehicles that are close enough to its player, according to the Characters’ and vehicles’ `NetCullDistanceSquared`._

## ActorInterestComponent

`ActorInterestComponent` is an Unreal Actor component that you add to an Actor. It contains:

* a list of queries
* a toggle for `NetCullDistanceSquared` (on by default)

> **Note**: You can only add one `ActorInterestComponent` to an Actor.

When a game client owns an Actor with the component `ActorInterestComponent`, the interest queries within this component control the information that the game client receives.

Each query within an `ActorInterestComponent` can consist of:

* a single constraint
* multiple constraints that you combine using `UOrConstraint`s and `UAndConstraint`s 

The following table lists the possible constraints.

| Constraint | Description |
| --- | --- |
| `UOrConstraint` | Satisfied if any of its inner constraints are satisfied. |
| `UAndConstraint` | Satisfied if all of its inner constraints are satisfied.|
| `USphereConstraint` | Includes all Actors within a sphere centered on the specified point. |
| `UCylinderConstraint` | Includes all Actors within a cylinder centered on the specified point. |
| `UBoxConstraint` | Includes all Actors within a bounding box centered on the specified point. |
| `URelativeSphereConstraint` | Includes all Actors within a sphere centered on the Actor that has `ActorInterestComponent`. |
| `URelativeCylinderConstraint` | Includes all Actors within a cylinder centered on the Actor that has `ActorInterestComponent`. |
| `URelativeBoxConstraint` | Includes all Actors within a bounding box centered on the Actor that has `ActorInterestComponent`. |
| `UCheckoutRadiusConstraint` | Includes all Actors of a class or a derived class within a cylinder centered on the Actor that has `ActorInterestComponent`. |
| `UActorClassConstraint` | Includes all Actors of a class. You can optionally include derived classes. |
| `UComponentClassConstraint` | Includes all Actors with an Unreal Actor component of a specified class. You can optionally include derived classes. |

For more information, see the [interest constraints header file](https://github.com/spatialos/UnrealGDK/blob/0.6.0-rc/SpatialGDK/Source/SpatialGDK/Public/Interop/SpatialInterestConstraints.h).

To attach this component to an Actor, open the Actor's Blueprint and, on the Components tab, select **Add Component**. Actor Interest is in the SpatialGDK section:

![Add component]({{assetRoot}}assets/screen-grabs/game-client-interest-management/add-component.png)
<br>_Image: Attach Actor Interest to an Actor_

### Example

![Set up interest]({{assetRoot}}assets/screen-grabs/game-client-interest-management/set-up-interest.png)
<br>_Image: The Interest section of the details panel for `ActorInterestComponent`_

In the screenshot above, `ActorInterestComponent` has two queries, so the game client receives updates about:

* all Pawns within 200 meters of the Actor that has `ActorInterestComponent` 
* all Actors of the class `Landmark` or derived from the class `Landmark`

## AlwaysInterested

You might want a game client to always receive updates about some specific Actors, regardless of the positions of the player and of these Actors. For example, you might want the game client to always receive updates about an important landmark, or about other members of the player’s team. You can use the `AlwaysInterested` `UPROPERTY` specifier for this.

An `AlwaysInterested` `UPROPERTY` must:

* be an object reference (either an AActor or UObject)
* have a [`Replicated`](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties/index.html) or [`Handover`]({{urlRoot}}/content/actor-handover) specifier

You set up an `AlwaysInterested` `UPROPERTY` on a game client’s PlayerController, or on any other Actor that the game client might own, and then you specify the Actor that you want the game client to always receive updates about.

### Example

For example, you might want a game client to always receive updates about a team base:

```
UPROPERTY(Replicated, AlwaysInterested)
AActor* TeamBase
```

If TeamBase is a valid Actor reference, then the game client receives updates about that Actor, regardless of TeamBase’s position in the game world.

Note: if the game client loses ownership of the Actor that you set up the `AlwaysInterested` `UPROPERTY` on, the `UPROPERTY` no longer applies. 

For example, you set up an `AlwaysInterested` `UPROPERTY` on a weapon, with a reference to the TeamBase. The game client owns the weapon only while its Pawn is holding the weapon. When the Pawn puts the weapon down, the game client stops receiving updates about the TeamBase.

<br/>
<br/>------------<br/>
_2019-07-31 Page added with editorial review._