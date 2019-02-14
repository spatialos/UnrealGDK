# Schema

SpatialOS uses [schema (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/concepts/schema#schema) to generate APIs specific to the entity components in your game world. These APIs define how you can operate on entity components within SpatialOS. 

## Generating schema

To generate schema, select the **Schema** button in the [GDK Toolbar]({{urlRoot}}/content/toolbars#buttons). The GDK automatically iterates through objects with replicated properties to generate the required schema files and then updates the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase).

![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)

_Image: In the GDK toolbar in the Unreal Editor, select **Schema**_

As the GDK automatically generates all the schema you need, you do not have to write or edit schema manually when using the GDK.

### When to generate schema

You must generate schema when you add or change any replicated properties that you want to deploy to SpatialOS.

The GDK only generates schema for objects currently loaded into memory. This means if your project uses [sublevels](<https://docs.unrealengine.com/en-us/Engine/Levels/LevelsWindow>), you’ll need to load them in addition to your map, before generating schema.

## Deleting schema

Whenever you generate schema, the GDK checks the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase) and all the relevant objects in your project and removes any references to missing objects from the SchemaDatabase. 

If you delete any objects in your project, the GDK removes them from the SchemaDatabase the next time you generate schema.

# Schema and source control 

### Checking out the `SchemaDatabase`

**Note:** If you are using the built-in [Unreal source control system](https://docs.unrealengine.com/en-US/Engine/UI/SourceControl) Unreal locks this file on checkout, meaning other users are unable to write to it. To prevent this, mark the `SchemaDatabase` file as writable locally on each machine, and only check out the file when you are ready to commit any changes made to it. 

## Schema for unique local objects

Whenever you generate schema, the GDK checks the `SchemaDatabase` and all the relevant objects in your project and removes any references to missing objects from the SchemaDatabase. 

This means that if you have an object that only exists on one user's machine (for example, a newly created object, or an object used for local testing) then these objects are automatically removed from the SchemaDatabase file whenever another user generates schema. 

To prevent this, commit newly created or modified classes to source control alongside the SchemaDatabase. 
