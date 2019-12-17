

# Port your project to SpatialOS

## 3. Launch a local deployment

Now you can launch a local deployment of your game. 
To do this: 

## Step 1. Make your project use SpatialOS networking

In the Unreal Editor, from the toolbar, open the **Play** drop-down menu and check two checkboxes:

  * Check the box for **Run Dedicated Server**
  * Check the box for **Spatial Networking**<br/>

![Play options]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-checkboxes.png)<br/>
_Image:The Unreal Engine **Play** drop-down menu, with **Run Dedicated Server** and **Spatial Networking** highlighted_

## Step 2. Launch a local deployment
Before you launch a local deployment, you must:

* generate schema (which creates entities)
* generate a snapshot

[block:html]
{
  "html": "<div class=\"wrap-collapsible\">\n  <input id=\"collapsible\" class=\"toggle\" type=\"checkbox\">
  <label for=\"collapsible\" class=\"lbl-toggle\">What is Schema? </label>\n  <div class=\"collapsible-content\">\n    <div class=\"content-inner\">\n      <p>\n </p>\n    </div>\n  </div>\n</div>"
}
[/block]


[block:html]
{
  "html": "<div class=\"wrap-collapsible\">\n  <input id=\"collapsible\" class=\"toggle\" type=\"checkbox\">
  <label for=\"collapsible\" class=\"lbl-toggle\">What is an entity? </label>\n  <div class=\"collapsible-content\">\n    <div class=\"content-inner\">\n      <p>\n </p>\n    </div>\n  </div>\n</div>"
}
[/block]


[block:html]
{
  "html": "<div class=\"wrap-collapsible\">\n  <input id=\"collapsible\" class=\"toggle\" type=\"checkbox\">
  <label for=\"collapsible\" class=\"lbl-toggle\">What is  a snapshot? </label>\n  <div class=\"collapsible-content\">\n    <div class=\"content-inner\">\n      <p>\n </p>\n    </div>\n  </div>\n</div>"
}
[/block]


To launch a local deployment:

1. In the Editor, on the [GDK Toolbar]({{urlRoot}}/content/unreal-editor-interface/toolbars), open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/>
   ![Schema]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Schema (Full Scan)**_
   </br>
1. Select [**Snapshot**]({{UrlRoot}}/content/spatialos-concepts/generating-a-snapshot) to generate a snapshot.<br/>
   ![Snapshot]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Snapshot**_</br>
1. Select **Start**. This opens a terminal window and starts a local SpatialOS deployment. Your game is ready to play when you see the output `SpatialOS ready` in your terminal window.</br>
   ![Start]({{assetRoot}}assets/screen-grabs/toolbar/start-button.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Start**_</br>
1. On the Editor toolbar, open the **Play** drop-down menu.
1. Under **Multiplayer Options**, set the number of players to **2** and ensure that the checkbox next to **Run Dedicated Server** is checked. (If it is unchecked, select the checkbox to enable it.)<br/>
   ![Multiplayer Options]({{assetRoot}}assets/set-up-template/spatialos-multiplayer-options.png)<br/>
   _Image: The Unreal Engine **Play** drop-down menu, with **Multiplayer Options** and **New Editor Window (PIE)** highlighted_</br>
1. Under **Modes**, select **New Editor Window (PIE)** to run the game. This starts one SpatialOS server-worker instance and two SpatialOS client-worker instances locally, in your Unreal Editor.
   The server-worker instance is acting as an Unreal server and the two client-worker instances are acting as two Unreal game clients (as would be used by two game players).
   You can switch between the two Editor windows to see and interact with each game client. 
1. If the game does not run automatically after selecting **New Editor Window (PIE)**, on the Editor toolbar, select **Play** to run the game.</br>
  ![Play]({{assetRoot}}assets/screen-grabs/toolbar/play-button.png)</br>
  _Image: On the Unreal Engine toolbar, select **Play**_</br>
1. When your game is running, select **Inspector** to open the [Inspector](https://docs.improbable.io/unreal/alpha//content/glossary#inspector) in your default web browser. The Inspector is a web-based tool that you use to explore the internal state of a SpatialOS world. It gives you a real-time view of what’s happening in a local or cloud deployment. </br>
  ![Inspector]({{assetRoot}}assets/screen-grabs/toolbar/inspector-button.png)</br>
  _Image: On the Unreal Engine toolbar, select **Inspector**_</br>
1. When you’re done, select **Stop** in the GDK toolbar to stop your local SpatialOS deployment.<br/>
   ![Stop]({{assetRoot}}assets/screen-grabs/toolbar/stop-button.png)<br/>
   _Image: On the GDK toolbar in the Editor, select **Stop**_</br>
</br>

**To run a local deployment with managed workers, or a cloud deployment, take a look at the [Starter Template guide]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro)**

You have now ported your Unreal game to GDK. 

If you have encountered any problems please check out our [troubleshooting]({{urlRoot}}/content/troubleshooting) and [known-issues]({{urlRoot}}/known-issues).

#### **> Next:** [Logs and modifications]({{urlRoot}}/content/tutorials/porting-guide/tutorial-portingguide-logs)

<br/>

<br/>------------<br/>_2019-08-02 Page updated with editorial review: updated project.<br/>
2019-07-16 Page updated with editorial review.<br/>
