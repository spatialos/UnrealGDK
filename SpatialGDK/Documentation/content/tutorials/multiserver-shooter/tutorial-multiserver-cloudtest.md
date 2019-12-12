

# \[Experimental\] Multiserver zoning
## 3: Test changes in the cloud
### Step 1: Build your workers

When you build your workers, you create an assembly. They’re `.zip` files that contain all the files that the SpatialOS Runtime uses when your run your game in the cloud. </br>
To do this you run a batch file,  `BuildWorker.bat`. You can add different flags to the script but, for now, run the script twice, once with a flag for server-workers and once with a flag for cloud-workers:</br>

  <%(#Expandable title="What flags are there?")%>
During development, you might want to, for example:</br> * cook a headless standalone version of the game ready for upload to the SpatialOS cloud as a simulated player cloud deployment, or </br> * cook a stand-alone version of the game to test it as a game client.</br> The optional flags give you this functionality and more.</br></br> See the [Helper script]({{urlRoot}}/content/apis-and-helper-scripts/helper-scripts) documentation for details of all the options and how to use them.
<%(/Expandable)%>

To build workers:

1. Close your Unreal Editor - if the Editor is open when you try to build workers, the command fails.
1. Check the project's networking is set to Spatial networking. </br>
GDK projects default to using Spatial networking. However, if you have reset the networking switch to native Unreal networking, you need to set it back. </br>
    1. In a terminal window, navigate to the `DefaultGame.ini` configuration file (located in `<ProjectRoot>\Game\Config` directory).
    2. Check if the `bSpatialNetworking` option is present. If it is, set it to `True` (so: `bSpatialNetworking=True`), save the file and exit. 
1. Still in a terminal window, navigate to the `UnrealGDKExampleProject` directory.
1. Run the `BuildProject.bat` command to build a server-worker using the filepath and flags below. </br>
The filepath you use depends on whether you have the `UnrealGDK` plugin set up as an *engine* plugin or as a *project* plugin. If you followed the default setup instructions which use the `InstallGDK.bat` script, you have it set up as an *engine* plugin. <br/></br>
      * Engine plugin filepath (default):</br>
      ```
      UnrealEngine\Engine\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooterServer Linux Development GDKShooter.uproject
      ```
      </br>
      * Project plugin filepath:</br>
      ```
      UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooterServer Linux Development GDKShooter.uproject
      ```
      </br></br>
  1. Now run the `BuildProject.bat` command to build a client-worker: <br/><br/>
      * Engine plugin filepath (default):</br>
      ```
      UnrealEngine\Engine\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooter Win64 Development GDKShooter.uproject
      ```
      <br/>
      * Project plugin filepath:</br>
      ```
        UnrealGDKExampleProject\Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat GDKShooter Win64 Development GDKShooter.uproject
      ```
      <br/><br/>

### Step 2: Upload your game

1. In File Explorer, navigate to `UnrealGDKExampleProject\spatial` and open `spatialos.json` in a text editor.
1. Change the `name` field to the name of your project. You can find this in the [Console](https://console.improbable.io). </br>It’ll be something like `beta_nuts_double_379`.
    ![]({{assetRoot}}assets/tutorial/project-name.png)
1. In a terminal window, change directory to `UnrealGDKExampleProject\spatial\` and run `spatial cloud upload <assembly_name>`, where `<assembly_name>` is a name of your choice (for example `myassembly`). A valid upload command looks like this:

```
spatial cloud upload myassembly
```

> **Note:** Depending on your network speed it may take a little while (1-10 minutes) to upload your assembly.

<br/>
### Step 3: Launch a cloud deployment

The next step is to launch a cloud deployment using the assembly that you just uploaded. This can only be done through the SpatialOS command-line interface (also known as the [SpatialOS CLI]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli).

When launching a cloud deployment you must provide three parameters:

* **the assembly name**, which identifies the worker assemblies to use.
* **a launch configuration**, which declares the world and load balancing configuration.
* **a name for your deployment**, which is used to label the deployment in the [Console]({{urlRoot}}/content/glossary#console).

1. In a  terminal window, in the same directory you used to upload your game, run: `spatial cloud launch --snapshot=snapshots/default.snapshot <assembly_name> two_worker_test.json <deployment_name>` 
    <br/>where `assembly_name` is the name you gave the assembly in the previous step and `deployment_name` is a name of your choice. A valid launch command would look like this:

```
spatial cloud launch --snapshot=snapshots/default.snapshot myassembly two_worker_test.json mydeployment
```

> **Note:** This command defaults to deploying to clusters located in the US. If you’re in Europe, add the `--cluster_region=eu` flag for lower latency.

<br/>
### Step 4: Play your game

![]({{assetRoot}}assets/tutorial/old-console.png)

When your deployment has launched, SpatialOS automatically opens the [Console](https://console.improbable.io) in your browser.

1. In the Console, Select the **Launch** button on the left of the page, and then click the **Launch** button that appears in the centre of the page. The SpatialOS Launcher, which was installed along with SpatialOS, downloads the game client for this deployment and runs it on your local machine.
    ![]({{assetRoot}}assets/tutorial/launch.png)
1. Once the client has launched, enter the game and fire a few celebratory shots - you are now playing in your first SpatialOS cloud deployment!

> **TIP:** Check out the [cloud deployment workflow page]({{urlRoot}}/content/cloud-deployment-workflow) for a reference diagram of this workflow.

<br/>
### Step 5: Invite your friends

1. To invite other players to this game, head back to the Deployment Overview page in your [Console](https://console.improbable.io), and select the **Share** button.
1. Share the generated link with your friends!

When you’re done shooting your friends, you can click the **Stop** button in the [Console](https://console.improbable.io) to stop your deployment.

<br/>
**Next steps:**

* If you want to create a new game using the GDK, you should get the [Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro). 
* If you want to port your existing game to the GDK, follow the [Porting guide]({{urlRoot}}/content/tutorials/tutorial-porting-guide).


<br/>------<br/>
_2019-08-14 Page updated with editorial review: added filepaths based on auto-install or manual-install._</br>
_2019-08-02 Page updated with limited editorial review: updated project name._</br>
_2019-03-20 Page updated with limited editorial review._
