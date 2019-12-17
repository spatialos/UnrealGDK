
# [Deprecated] Multiple deployments for session-based games
## 3: Build and upload workers 

### Step 1: Edit the .ini file

Before you build your [assembly]({{urlRoot}}/content/glossary#assembly), you need to edit the Example Project `DefaultEngine.ini` file. To do this:

1. In File Explorer, navigate to `UnrealGDKExampleProject\Game\Config` and open `DefaultEngine.ini` in a text editor.
1. In your text editor, search for `bPreventAutoConnectWithLocator=False` and change this value to `True`. This setting forces your game client to stay offline after being launched until you select a deployment or select Quick Join in the game. Your changes should look like this:
    
    [block:code]
{
  "codes": [
  {
      "code": "    [/Script/SpatialGDK.SpatialGameInstance] \n bPreventAutoConnectWithLocator=True\n",
      "language": "text"
    }
  ]
}
[/block]
1. Save and close `DefaultEngine.ini` once you have made your changes. 

> **Note**: You must close the Unreal Editor before building your workers. If the Editor is open when you try to build your workers, the command will fail.
</br>
</br>

### Step 2: Build your assembly

There are two ways to build workers for your assembly, you can either:

[block:html]
{
  "html": "<div class=\"wrap-collapsible\">\n  <input id=\"collapsible\" class=\"toggle\" type=\"checkbox\">
  <label for=\"collapsible\" class=\"lbl-toggle\">Build your workers using `BuildProject.bat` </label>\n  <div class=\"collapsible-content\">\n    <div class=\"content-inner\">\n      <p>\n </p>\n    </div>\n  </div>\n</div>"
}
[/block]


or

[block:html]
{
  "html": "<div class=\"wrap-collapsible\">\n  <input id=\"collapsible\" class=\"toggle\" type=\"checkbox\">
  <label for=\"collapsible\" class=\"lbl-toggle\">Build your workers using terminal commands </label>\n  <div class=\"collapsible-content\">\n    <div class=\"content-inner\">\n      <p>\n </p>\n    </div>\n  </div>\n</div>"
}
[/block]


**Note:** Building your workers can take a long time, regardless of which method you use. 

**Troubleshooting**</br>


[block:html]
{
  "html": "<div class=\"wrap-collapsible\">\n  <input id=\"collapsible\" class=\"toggle\" type=\"checkbox\">
  <label for=\"collapsible\" class=\"lbl-toggle\">BuildProject.bat can’t find the path specified </label>\n  <div class=\"collapsible-content\">\n    <div class=\"content-inner\">\n      <p>\n </p>\n    </div>\n  </div>\n</div>"
}
[/block]


### Step 3: Upload your assembly

Before launching a cloud deployment, you must upload your assembly, which contains your server-worker and client-worker. To do this:

1. Open a terminal window and navigate to `\UnrealGDKExampleProject\spatial`.
1. Run the following SpatialOS CLI command: `spatial cloud upload <assembly_name>`
<br/>Where `<assembly_name>` is a name you create here for the upload (for example “myassembly”).

Make a note of the `<assembly_name>` you create, as you will need it later. 

</br>
</br>
### **> Next**: [4. Configure the Deployment Manager]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-configure)


<br/>------<br/>
_2019-10-16 Tutorial deprecated_<br/>
_2019-05-21 Page added with editorial review_
