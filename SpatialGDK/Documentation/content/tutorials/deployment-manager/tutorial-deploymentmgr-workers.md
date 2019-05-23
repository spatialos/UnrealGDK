# Multiple deployments for session-based games
## 3: Build and upload workers 

Before you build your worker assemblies, you need to edit the Example Project `DefaultEngine.ini` file.

In File Explorer, navigate to `UnrealGDKExampleProject\Game\Config` and open `DefaultEngine.ini` in a text editor. 

In your text editor, search for `bPreventAutoConnectWithLocator=False` and change this value to `True`. 

This setting forces your game client to stay offline after being launched until you select a deployment or select Quick Join in the game. 

```
[/Script/SpatialGDK.SpatialGameInstance]
bPreventAutoConnectWithLocator=True
```

Save and close `DefaultEngine.ini` once you have made your changes. 

> **Note**: You must close the Unreal Editor before building your workers. If the Editor is open when you try to build your workers, the command will fail.
</br>
</br>


### Step 1: Build your worker assemblies

There are two ways to build your worker assemblies (known as “building workers”):

- Build your workers automatically using the `BuildProject.bat` script. </br>

This script automatically builds both the server-workers and client-workers required to run your game in the cloud. It then compresses your workers and saves them as `.zip` files to the `UnrealGDKExampleProject\spatial\build\assembly\worker` directory. Use this script if you want to build server-workers and client-workers at the same time. 

- Build your workers manually using the command line. </br>

Use the command line when you want to build your server-workers and client-workers separately, or, if you want to build different worker configurations.

**Note:** Building your workers can take a long time, regardless of which method you use. 

<%(#Expandable title="Build your workers using `BuildProject.bat`")%>

In File Explorer, navigate to the `UnrealGDKExampleProject` directory.

Double click `BuildProject.bat`. This opens a command line window and automatically builds your client and server workers. 

<%(/Expandable)%>

<%(#Expandable title="Build your workers using terminal commands")%>

In a terminal window, navigate to the `UnrealGDKExampleProject` directory.

1. Build a server-worker assembly by running the following command: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooterServer Linux Development GDKShooter.uproject`
2. Build a client-worker assembly by running the following command: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooter Win64 Development GDKShooter.uproject`

<%(/Expandable)%>

#### Troubleshooting

<%(#Expandable title="BuildProject.bat can’t find the path specified")%>

If you receive the error `The system cannot find the path specified. Builds failed.`, open the `ProjectPaths.bat` file in a text editor and ensure that `PROJECT_PATH` and `GAME_NAME` are correct. `PROJECT_PATH` needs to be the name of your Unreal project folder (usually “Game”). `GAME_NAME` needs to be the same name as your Unreal Project `.uproject` file.  

<%(/Expandable)%>

### Step 2: Upload your worker assemblies

Before launching a cloud deployment, you must upload your server-worker and client-worker assemblies to the cloud. To do this: 

1. Open a terminal window and navigate to `\UnrealGDKExampleProject\spatial`.
2. Run the following SpatialOS CLI command: `spatial cloud upload <assembly_name>`
<br/>Where `<assembly_name>` is a name you create here for the upload (for example “myassembly”).

Make a note of the `<assembly_name>` you create, as you will need it later. 

</br>
</br>
**> Next**: [4. Configure the Deployment Manager]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-configure)

--------<br/>

_2019-05-21 Page added with editorial review_