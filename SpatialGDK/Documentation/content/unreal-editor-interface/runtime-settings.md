<toc>
# SpatialOS Runtime Settings panel

Before you launch your game in a [cloud deployment]({{urlRoot}}/content/glossary#deployment), you need to set up its [SpatialOS Runtime]({{urlRoot}}/content/glossary##spatialos-runtime) settings. You can use the default settings or change them in the Runtime Settings panel.

## Access the SpatialOS Runtime settings

* From the Unreal Editor:
    * From the **Edit** menu:
        0. Select **Project Settings** to display the Project Settings editor.
        0.  Scroll down to **SpatialOS GDK for Unreal** and select **Runtime Settings** to display the SpatialOS Runtime Settings panel.
* From the Unreal Toolbar:
	0. Select **Play** > **SpatialOS Settings** to display the Project Settings editor. 
	0. Scroll down to **SpatialOS GDK for Unreal** and select **Runtime Settings** to display the SpatialOS Runtime Settings panel.
* From the file system: The SpatialOS Runtime settings are in the `DefaultSpatialGDKSettings.ini` file in the `<GameRoot>\Config\` directory, which you can edit with any text editor.<br/>

<%(Lightbox image="{{assetRoot}}assets/editor-settings/runtime-settings.png")%> 

<%(Callout type="tip" message="Each setting is documented with a tooltip which appears when you hover over it in the editor.")%>

## Example use cases

In the following use cases, you use the SpatialOS Runtime Settings panel to configure your project:

* You know that the number of Actors that you need the SpatialOS Runtime to reserve entity IDs for at the start of your game deployment is greater than the default 3,000: Increase the value of **Initial entity IDs reserved** to the number of Actors the game needs as it starts up.
* The SpatialOS Runtime is not reserving a large enough batch of new entity IDs quickly enough during your game’s Runtime: Increase the value of the **Pool refresh threshold** so that the SpatialOS Runtime is triggered to reserve new batch of entity IDs sooner.

<br/>
<br/>------------<br/>
_2019-07-31 Page added with limited editorial review._
<!-- Ticket: https://improbableio.atlassian.net/browse/DOC-1227 -->