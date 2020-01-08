<%(TOC)%>

# How to use schema

## When to generate schema  
Your SpatialOS deployment relies upon schema, which the GDK generates for you, so you do not have to write or edit the schema manually. You should request it to generate (or regenerate) schema whenever you add or change any [replicated properties (Unreal documentation)](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties) or [RPCs (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs).

## How to generate schema  
There are several ways to generate schema for your project: 

* Full scan from within the Unreal Editor or via the command line. Use this method when you haven't generated any schema for your project, or you've added or edited a class with replicated properties or RPCs that is not currently loaded in the Editor
* Iterative generation from within the Unreal Editor for classes with replicated properties or RPCs that are currently loaded in the Editor. Use this method when you have added or edited a class with replicated properties or RPCs that is currently loaded in the Editor
* During cooking via a commandlet, which is like running the full scan described above, but is faster. This method is still experimental.


For large projects, the full scan methods are significantly slower than the iterative method.

> **Note:** Whenever you generate schema, the GDK creates schema for you, for classes with replicated properties or RPCs. It stores schema in your project's [`SchemaDatabase`]({{urlRoot}}/content/glossary#schemadatabase) file. 
> <br><br>
> If a class does not have replicated properties or RPCs, then the GDK does not generate schema for it.

### Generate schema from within the Unreal Editor  
Within the Unreal Editor, you can generate schema using a full scan, or generate schema iteratively. For information on when to use each method, see [How to generate schema](#how-to-generate-schema).

* **Full scan** 

    To generate schema for all classes in your project that have replicated properties or RPCs:<br/>

    In the Unreal Editor, on the [GDK toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars#buttons), open the **Schema** drop-down menu and select **Schema (Full Scan)**.<br/> You must select **Schema (Full Scan)** the first time you generate schema for a project. 
    <br/> ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
    _Image: In the GDK toolbar in the Unreal Editor, select **Schema (Full Scan)**_<br/>
    <br/>When you select **Schema (Full Scan)**, the GDK searches through all classes in your project that have replicated properties or RPCs to generate the schema files, and then updates the `SchemaDatabase`. <br/>

    Run a full scan the first time you generate schema for your project, and whenever you need to generate schema for classes that have replicated properties or RPCs but that are not currently loaded in the Editor.<br/>For example: You need to select **Schema (Full Scan)** if you haven’t generated schema after adding a new Blueprint to your game, and that Blueprint is no longer open in the Editor.<br/><br/>

* **Iterative**

    To generate schema for classes that have replicated properties or RPCs and that are currently loaded in the Editor: <br/>

    Select **Schema** in the [GDK toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars#buttons). The GDK searchesthrough classes that have replicated properties or RPCs and that are currently loaded in the Editor, generates the schema files and updates the `SchemaDatabase`.<br/>
    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)<br/>
    _Image: In the GDK toolbar in the Unreal Editor, select **Schema**_<br/>

### Generate schema via the command line  
You can generate schema via the command line using an Unreal commandlet. This method uses a [full scan](#how-to-generate-schema)). It's useful as part of an automated build system.

To generate schema via the command line, run the following command:

```
<Path to UE4Editor.exe> <Path to your game's .uproject file> -run=GenerateSchemaAndSnapshots -MapPaths=<MapName1;MapName2;MapName3>
```

This command generates schema using a full scan for all classes that have replicated properties or RPCs, and generates individual [snapshots]({{urlRoot}}/content/glossary#snapshot) (`<MapName>.snapshot`) for each map. Any specified map paths that end in `/` are interpreted as a directory, and snapshots are generated for each `.umap` found under these paths.


### Generate schema during the cooking process  
The `CookAndGenerateSchemaCommandlet` command generates the schema needed by your game, and does so during the cooking process, so it is like running the full scan described above, but is faster. It will ultimately replace the command line schema generation described above, but is currently experimental and in ‘alpha’ state.

#### How it works  
`CookAndGenerateSchemaCommandlet` is a subclass of the native `CookCommandlet`. When it runs, it triggers an Unreal-native cook and supplies it with the necessary arguments. During the cook, it keeps track of all classes loaded that require schema. After the cook, schema are generated for those supported classes.

#### Run the command  
You run the commandlet in the same way you would run the Unreal-native [`Cook` commandlet](https://docs.unrealengine.com/en-US/Engine/Deployment/Cooking/index.html), with the appropriate arguments, by invoking `-run=CookAndGenerateSchema` instead of `-run=Cook`: 

```
<Path to UE4Editor-Cmd.exe> <Path to your game's .uproject file> -run=CookAndGenerateSchema -TargetPlatform=WindowsNoEditor -fileopenlog -unversioned -stdout -CrashForUAT -unattended -NoLogTimes  -UTF8Output


```


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
_2020-01-08 Page updated with editorial review: added commandlet_<br/>
_2019-07-26 Page updated with editorial review_