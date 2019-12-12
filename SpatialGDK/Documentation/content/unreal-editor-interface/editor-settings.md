
# SpatialOS Editor Settings panel
In the [Project Settings](https://docs.unrealengine.com/en-us/Engine/UI/ProjectSettings) editor, you can use the SpatialOS Editor Settings panel to change the default settings for local deployments, schema generation, and snapshot generation. For example, you might want to use the panel to extend the size of your [game world]({{urlRoot}}/content/glossary#game-world).

## Access the panel
To access the **SpatialOS Editor Settings** panel in the Unreal Editor, use either of the following methods:

* From the **Edit** menu:
	1. Select **Project Settings** to display the Project Settings editor.
	2.  Scroll down to **SpatialOS GDK for Unreal** and select **Editor Settings**.


* From the Unreal Toolbar:
	1. Select **Play** > **SpatialOS Settings** to display the Project Settings editor. 
	2. Scroll down to **SpatialOS GDK for Unreal** and select **Editor Settings**. 

  <%(Lightbox image="{{assetRoot}}assets/editor-settings/spatialos-settings.png")%> 

Note that the default SpatialOS editor settings for your project are saved in the `DefaultSpatialGDKEditorSettings.ini file` in the `<GameRoot>\Config\` directory, which you can edit with any text editor.

[block:callout]
{
  "type": "tip",
  "body": "Each setting is documented with a tooltip which appears when you hover over it in the editor."
}
[/block]

## Example use cases

In the following use cases, you use the SpatialOS Editor Settings panel to configure your project:

* To make your game world larger than the default setting of 2km by 2km, you change the values of dimensions for your game world. Alternatively, write this directly in your game project's [launch configuration file]({{urlRoot}}/content/glossary#launch-configuration-file).
* To configure the [rectangular grid](https://docs.improbable.io/reference/latest/shared/worker-configuration/load-balancing#rectangular-grid-rectangle-grid) load balancing strategy when you're testing your multiserver game, you specify a rectangle grid column and a row count. Alternatively, write this directly in your game project’s [launch configuration file]({{urlRoot}}/content/glossary#launch-configuration-file).


<br/>
<br/>------------<br/>
_2019-07-31 Page updated with limited editorial review._
<!-- Ticket: https://improbableio.atlassian.net/browse/DOC-1227 -->
