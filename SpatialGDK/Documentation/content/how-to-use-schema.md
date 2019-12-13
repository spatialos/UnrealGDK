

# How to use schema

## When to generate schema

You must generate schema when you add or change any [replicated properties (Unreal documentation)](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties) or [RPCs (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs) that you want to deploy to SpatialOS.

## How to generate schema

There are three ways to generate schema for your project: two that use a full scan (one within the Unreal Editor, and one via the command line) and one that is iterative (within the Unreal Editor).

For large projects, the full scan methods are significantly slower than the iterative method.

See the table below for when to use each method.

| Situation | Method to use |
|---|---|
| Either: <ul><li>you haven't generated any schema for your project, or</li><li>you've added or edited a class with replicated properties or RPCs that is not currently loaded by the Editor</li></ul> | Full scan within the Unreal Editor or via the command line |
| You've added or edited a class with replicated properties or RPCs that is currently loaded by the Editor | Iterative within the Unreal Editor |

> **Note:** Whenever you generate schema, the GDK creates schema for you, for classes with replicated properties or RPCs. It stores shema in your project's [`SchemaDatabase`]({{urlRoot}}/content/glossary#schemadatabase) file. 
> <br><br>
> If a class does not have replicated properties or RPCs, then the GDK does not generate schema for it.

### Generating schema within the Unreal Editor
Within the Unreal Editor, you can generate schema using a full scan, or generate schema iteratively. For information on when to use each method, see [How to generate schema](#how-to-generate-schema).

* **Full scan** 

    To generate schema for all classes in your project that have replicated properties or RPCs:<br/>

    In the Unreal Editor, on the [GDK toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars#buttons), open the **Schema** drop-down menu and select **Schema (Full Scan)**.<br/> You must select **Schema (Full Scan)** the first time you generate schema for a project. 
    <br/> ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
    _Image: In the GDK toolbar in the Unreal Editor, select **Schema (Full Scan)**_<br/>
    <br/>When you select **Schema (Full Scan)**, the GDK iterates through all classes in your project that have replicated properties or RPCs to generate the schema files, and then updates the `SchemaDatabase`. <br/>

    Run a full scan the first time you generate schema for your project, and whenever you need to generate schema for classes that have replicated properties or RPCs but that are not currently loaded by the Editor.<br/>For example: You need to select **Schema (Full Scan)** if you didn't generate schema after adding a new Blueprint to your game, and that Blueprint is no longer open in the Editor.<br/><br/>

* **Iterative**

    To generate schema for classes that have replicated properties or RPCs and that are currently loaded by the Editor: <br/>

    Select **Schema** in the [GDK toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars#buttons). The GDK iterates through classes that have replicated properties or RPCs and that are currently loaded by the Editor, generates the schema files and updates the `SchemaDatabase`.<br/>
    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)<br/>
    _Image: In the GDK toolbar in the Unreal Editor, select **Schema**_<br/>

### Generating schema via the command line
You can generate schema via the command line using an Unreal commandlet. This method uses a full scan (see [How to generate schema](#how-to-generate-schema)). It's useful as part of an automated build system.

To generate schema via the command line, run the following command:

[block:code]
{
  "codes": [
  {
    "code": "<Path to UE4Editor.exe> <Path to your game's .uproject file> -run=GenerateSchemaAndSnapshots -MapPaths=<MapName1;MapName2;MapName3>",
     "language": "text"
    }
  ]
}
[/block]

This command generates schema using a full scan for all classes that have replicated properties or RPCs, and generates individual [snapshots]({{urlRoot}}/content/glossary#snapshot) (`<MapName>.snapshot`) for each map. Any specified map paths that end in `/` are interpreted as a directory, and snapshots are generated for each .umap found under these paths.

As the GDK automatically generates all the schema you need, you do not have to write or edit schema manually when using the GDK.

## How to delete schema

When you generate schema, the GDK verifies that any classes referenced in the `SchemaDatabase` still exist. If you delete a class, the GDK removes it from the `SchemaDatabase` the next time you generate schema.

## How to exclude directories from schema

To exclude directories from schema generation, add them to `Directories to never cook`. This can be done within the Unreal Editor under **Project Settings > Project Packaging > Packaging > Directories to never cook**. 

Note that you will not be able to use assets without generated schema in a Spatial deployment, so make sure to exclude only those directories that do not store assets that you are using in your game.

## Schema and source control

If you are using the built-in [Unreal source control system](https://docs.unrealengine.com/en-US/Engine/UI/SourceControl) Unreal locks this file on checkout, meaning other users are unable to write to it. To prevent this, mark the `SchemaDatabase` as writable locally on each machine, and only check out the file when you are ready to commit any changes made to it.

## Schema for unique local classes

Whenever you generate schema, the GDK checks the [`SchemaDatabase`]({{urlRoot}}/content/glossary#schemadatabase) and all the in-memory classes in your project and removes any classes referenced in the `SchemaDatabase` that no longer exist.

This means that if you have a class that only exists on one user's machine (for example, a newly created class, or a class used for local testing) then these classes are automatically removed from the `SchemaDatabase` whenever another user generates schema.

To prevent this, commit newly created or modified classes to source control alongside the `SchemaDatabase`.

<br/>------------<br/>
_2019-07-26 Page updated with editorial review_
