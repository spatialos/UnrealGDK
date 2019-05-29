<%(TOC)%>
# SpatialOS concepts: world, entities, components

> **Tip:** Before you read this page, you should read [What is SpatialOS?]({{urlRoot}}/content/spatialos-concepts/what-is-spatialos)

## The SpatialOS game world

In a game running on SpatialOS, the SpatialOS game world is a core concept. It’s the canonical source of truth about things in your game.

## Entities and components

Entities are the objects in your game. All of the data that you want to share between SpatialOS game servers has to be stored in SpatialOS entities. Each entity is made up of SpatialOS components; it's the components which store this data, in their properties.

> Entities equate to Actors in Unreal. SpatialOS components’ properties equate to replicated properties in Unreal. Note that SpatialOS components are not the same as Components in Unreal.

For example, in a world with rabbits and lettuces, you'd have the SpatialOS entities  `Rabbit` and `Lettuce`, each with certain SpatialOS components. These components in turn would have certain properties:

![Entities example]({{assetRoot}}assets/screen-grabs/component-details.png)

_Image: Entities, components, and component properties_

## Why is a SpatialOS game world necessary?

The SpatialOS game world, with its entities and their components, stores the state of the game world in a way that enables many servers and game clients to access and change it, without needing to communicate directly with each other.