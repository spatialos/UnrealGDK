<%(TOC)%>

# How to use schema

## When to generate schema

You must generate schema when you add or change any [replicated properties (Unreal documentation)](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties) that you want to deploy to SpatialOS.

## How to generate schema

There are two ways to generate schema for your project:

* **Generate schema (Full Scan)** 

    To generate schema for all classes* in your project:<br/>

    In the Unreal Editor, on the [GDK Toolbar]({{urlRoot}}/content/toolbars#buttons), open the **Schema** drop-down menu and select **Schema (Full Scan)**.<br/> You must select **Schema (Full Scan)** the first time you generate schema for a project. 
    <br/> ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
    _Image: In the GDK toolbar in the Unreal Editor, select **Schema (Full Scan)**_<br/>
    <br/>When you select **Schema (Full Scan)**, the GDK automatically iterates through all classes* in your project to generate the schema files, and then updates the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase). <br/>

    Run a full scan the first time you generate schema for your project, and whenever you need to generate schema for classes* that are not currently loaded by the Editor.<br/>For example: You need to select **Schema (Full Scan)** if you didn't generate schema after adding a new Blueprint to your game, and that Blueprint is no longer open in the Editor.<br/><br/>

* **Generate Schema (Iterative)**

    To generate schema for classes* that are currently loaded by the Editor: <br/>

    Select **Schema** in the [GDK Toolbar]({{urlRoot}}/content/toolbars#buttons). The GDK automatically iterates through classes* that are currently loaded by the Editor, generates the schema files and updates the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase).<br/>
    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)<br/>
    _Image: In the GDK toolbar in the Unreal Editor, select **Schema**_<br/><br/>

As the GDK automatically generates all the schema you need, you do not have to write or edit schema manually when using the GDK.

> \*Whenever you generate schema, the GDK automatically creates schema for classes with [replicated properties (Unreal documentation)](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties) or [RPCs (Unreal documentation)](https://docs.unrealengine.com/en-us/Gameplay/Networking/Actors/RPCs). If a class does not have replicated properties or RPCs, then the GDK does not generate schema for it. 

## When to generate schema

Select **Schema (Full Scan)** if you have: 

* Not generated any schema for your project.
* Added a class with replicated properties that is not currently loaded by the Editor.
* Edited a class with replicated properties that is not currently loaded by the Editor.

Select **Schema** if you have:

* Added a class with replicated properties that is currently loaded by the Editor.
* Edited a class with replicated properties that is currently loaded by the Editor.

## How to delete schema

When you generate schema, the GDK verifies that any classes referenced in the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase) still exist. If you delete a class, the GDK removes it from the SchemaDatabase the next time you generate schema.

## Schema and source control

If you are using the built-in [Unreal source control system](https://docs.unrealengine.com/en-US/Engine/UI/SourceControl) Unreal locks this file on checkout, meaning other users are unable to write to it. To prevent this, mark the `SchemaDatabase` file as writable locally on each machine, and only check out the file when you are ready to commit any changes made to it.

## Schema for unique local classes

Whenever you generate schema, the GDK checks the [SchemaDatabase]({{urlRoot}}/content/glossary#schemadatabase) and all the in-memory classes in your project and removes any classes referenced in the `SchemaDatabase` that no longer exist.

This means that if you have a class that only exists on one user's machine (for example, a newly created class, or a class used for local testing) then these classes are automatically removed from the SchemaDatabase file whenever another user generates schema.

To prevent this, commit newly created or modified classes to source control alongside the SchemaDatabase.