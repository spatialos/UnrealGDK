<%(TOC)%>
# Schema

SpatialOS uses [schema](https://docs.improbable.io/reference/latest/shared/concepts/schema#schema) to generate APIs specific to the components in your project. These APIs define how you can operate on entity components within SpatialOS.

## Generating schema

To generate schema, select the **Schema** button in the [GDK Toolbar]({{urlRoot}}/content/toolbars#buttons). The GDK automatically iterates through classes with replicated properties to generate the required schema files and then updates the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase).

![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)

_Image: In the GDK toolbar in the Unreal Editor, select **Schema**_

As the GDK automatically generates all the schema you need, you do not have to write or edit schema manually when using the GDK.

### When to generate schema

You must generate schema when you add or change any [replicated properties (Unreal documentation)](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties) that you want to deploy to SpatialOS.

The GDK only generates schema for classes currently loaded into memory. This means if your project uses [sublevels](<https://docs.unrealengine.com/en-us/Engine/Levels/LevelsWindow>), you’ll need to load them in addition to your map, before generating schema.

## Deleting schema

When you generate schema, the GDK verifies that any classes referenced in the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase) still exist. If you delete a class, the GDK removes it from the SchemaDatabase the next time you generate schema. 
# Schema and source control 

### Checking out the `SchemaDatabase`

**Note:** If you are using the built-in [Unreal source control system](https://docs.unrealengine.com/en-US/Engine/UI/SourceControl) Unreal locks this file on checkout, meaning other users are unable to write to it. To prevent this, mark the `SchemaDatabase` file as writable locally on each machine, and only check out the file when you are ready to commit any changes made to it. 

## Schema for unique local classs

Whenever you generate schema, the GDK checks the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase) and all the in-memory classes in your project and removes any classes referenced in the `SchemaDatabase` that no longer exist.

This means that if you have a class that only exists on one user's machine (for example, a newly created class, or a class used for local testing) then these classes are automatically removed from the SchemaDatabase file whenever another user generates schema. 

To prevent this, commit newly created or modified classes to source control alongside the SchemaDatabase. 
