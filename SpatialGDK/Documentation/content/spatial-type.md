# Spatial Type

To run Unreal-developed games in SpatialOS, we need to expose network-relevant class information to SpatialOS. Spatial Type (`SpatialType`) is a SpatialOS-specific [class specifier (Unreal documentation)](https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Reference/Classes/Specifiers) which the GDK uses to reflect this information. `SpatialType` is similar to other Unreal class specifiers, but implemented in parallel with [EClassFlags (Unreal documentation)](https://api.unrealengine.com/INT/API/Runtime/CoreUObject/UObject/EClassFlags/index.html) to minimize the possibility of conflicting changes between the standard Unreal Engine and the GDK’s Unreal Engine fork.

The `SpatialType` tag allows the GDK to interoperate between the network stacks of native Unreal and SpatialOS. The tag is not inherited down class hierarchies.

**Note:** For your Actor to replicate across SpatialOS it needs to have the `SpatialType` tag. 

## Classes with automatic `SpatialType` tagging
You don’t need to manually tag all classes as a `SpatialType` because the GDK automatically detects and tags classes that are likely to be replicated.
The auto-detection and tagging is a setting in the GDK toolbar, checked by default. This setting ensures the GDK inspects all loaded classes and tags them with `SpatialType` if they have any properties tagged with `Replicated` or `Handover`. 


### How to disable automatic tagging
Although automatic tagging is enabled by default, you can disable it. 
<br/>To do this: 
1. From the Unreal Editor menu, go to: **Edit** > **Project Settings** > **SpatialOS Unreal GDK** and click on  **Toolbar** to bring up the **SpatialOS Unreal GDK - Toolbar** configuration dialog box.
1. In the dialogue box, under **Schema Generation**, uncheck the box against **Generate schema for all supported classes**.


**Note:** As the GDK is in alpha, there is a small chance that the schema won’t function correctly for a specific class. Being able to disable automatic tagging and manually specifying all your `SpatialType` classes allows you to work around this.

## Classes which need manual `SpatialType` tagging
You need to manually tag as `SpatialType` any classes which are [Singleton Actors]({{urlRoot}}/content/singleton-actors). These Singleton Actor classes also need `SpatialType` descriptors. 

### `SpatialType` descriptors
You can add descriptors to the `SpatialType` tag to define additional information SpatialOS needs to know about your Unreal class.
These are:
`Singleton` - this indicates this class should be treated as a Singleton. .
`ServerOnly` - this indicates this class is only relevant to [server-workers]({{urlRoot}}/content/glossary#workers). You only use this descriptor in conjunction with the Singleton descriptor; it indicates that this class is a Private Singleton.

## How to manually tag classes as `SpatialType`

# Adding the specifier and descriptors to your Unreal C++ class

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

# Adding the specifier and descriptors to your Unreal Blueprint class
You can also tag your Blueprint classes with the `SpatialType` tag and descriptors. To do this,  
1. Open the class in the Blueprint Editor and, from the menu, select **Class Settings**. 
1. Select **Class Options** and under **Advanced**, check the `Spatial Type` checkbox. 
1. Add any descriptors to the `Spatial Description` textbox.

[Using Blueprints to manually add a Singleton descriptor for a `SpatialType`]({{assetRoot}}assets/screen-grabs/blueprint_singleton.png)
