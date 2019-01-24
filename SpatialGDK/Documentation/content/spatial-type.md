# Spatial Type

 Spatial Type (`SpatialType`) is a SpatialOS-specific [class specifier (Unreal documentation)](https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Reference/Classes/Specifiers). The GDK uses `SpatialType` to expose network-relevant class information to SpatialOS.
 
 `SpatialType` is similar to other Unreal class specifiers, but implemented in parallel with [EClassFlags (Unreal documentation)](https://api.unrealengine.com/INT/API/Runtime/CoreUObject/UObject/EClassFlags/index.html) to minimize the possibility of conflicting changes between the standard Unreal Engine and the GDK’s Unreal Engine fork.

The `SpatialType` tag allows the GDK to interoperate between the network stacks of native Unreal and SpatialOS. The tag is inherited down class hierarchies.

## Classes with automatic SpatialType tagging
By default all classes are tagged with `SpatialType`. This ensures the GDK inspects all loaded classes and generates schema for them if they have any properties tagged with `Replicated` or `Handover`, or they have RPC functions. If you don't wish your class to be considered a `SpatialType` you can opt out using the `NotSpatialType` tag.

## Classes which need manual SpatialType tagging
You need to manually tag as `SpatialType` any classes which are [Singleton Actors]({{urlRoot}}/content/singleton-actors) or only accessible to [server-workers]({{urlRoot}}/content/glossary#workers). These classes also need `SpatialType` descriptors. 

### SpatialType descriptors
You can add descriptors to the `SpatialType` tag to define additional information SpatialOS needs to know about your Unreal class.
These are:

* `Singleton`: this indicates this class should be treated as a Singleton.
* `ServerOnly`: this indicates this class is only relevant to [server-workers]({{urlRoot}}/content/glossary#workers). You can use this descriptor in conjunction with the `Singleton` descriptor to indicate that this class is a Private Singleton.

## How to manually tag classes as SpatialType

### Adding the specifier and descriptors to your Unreal C++ class

Like other Unreal class specifiers, you specify the `SpatialType` and descriptors within your class’s `UCLASS` macro. 

For example;

```
UCLASS(SpatialType)
class AMyReplicatingActor : public AActor
{
  GENERATED_BODY()
  ...
}
```

To add `SpatialType` descriptors, use the following format;

```
UCLASS(SpatialType=Singleton)
class AMySingleton : public AActor
{
  GENERATED_BODY()
  ...
}
```

### Adding the specifier and descriptors to your Unreal Blueprint class
You can also tag your Blueprint classes with the `SpatialType` tag and descriptors. To do this,

1. Open the class in the Blueprint Editor and, from the menu, select **Class Settings**. 
1. Select **Class Options** and under **Advanced**, check the `Spatial Type` checkbox. 
1. Add any descriptors to the `Spatial Description` textbox, like so:

![blueprint-singleton]({{assetRoot}}assets/screen-grabs/blueprint-singleton.png)
