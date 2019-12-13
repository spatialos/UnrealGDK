
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
  "html": "<button class=\"collapsible\">Build your workers using `BuildProject.bat`</button>\n<div>\n\n\n1. In File Explorer, navigate to the `UnrealGDKExampleProject` directory.\n1. Double click `BuildProject.bat`. This opens a command line window and automatically builds your client and server workers.\n\nThis script automatically builds both the [server-workers]({{urlRoot}}/content/glossary#server-workers) and [client-workers]({{urlRoot}}/content/glossary#client-workers) required to run your game in the cloud. It then compresses your workers and saves them as `.zip` files to the `UnrealGDKExampleProject\spatial\build\assembly\worker` directory. Use this script if you want to build server-workers and client-workers at the same time.\n</div>"
}
[/block]


or

[block:html]
{
  "html": "<button class=\"collapsible\">Build your workers using terminal commands</button>\n<div>\n\n1. In a terminal window, navigate to the `UnrealGDKExampleProject` directory.\n1. Build a server-worker by running the following command: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooterServer Linux Development GDKShooter.uproject`\n1. Build a client-worker by running the following command: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooter Win64 Development GDKShooter.uproject`\n\n\nUse the command line when you want to build your [server-workers]({{urlRoot}}/content/glossary#server-workers) and [client-workers]({{urlRoot}}/content/glossary#client-workers) separately, or, if you want to build different worker configurations.\n\n\n</div>"
}
[/block]


**Note:** Building your workers can take a long time, regardless of which method you use. 

**Troubleshooting**</br>


[block:html]
{
  "html": "<button class=\"collapsible\">BuildProject.bat can’t find the path specified</button>\n\n<div>\n\n\nIf you receive the error `The system cannot find the path specified. Builds failed.`, open the `ProjectPaths.bat` file in a text editor and ensure that `PROJECT_PATH` and `GAME_NAME` are correct. `PROJECT_PATH` needs to be the name of your Unreal project folder (usually “Game”). `GAME_NAME` needs to be the same name as your Unreal Project `.uproject` file.  \n\n\n</div>"
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
