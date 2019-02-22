# Get started: Port your own Unreal project to the GDK

As an experienced UE4 developer, you likely have a prototype or a game already. The GDK allows you to port it over to SpatialOS. This guide shows you how to kickstart your SpatialOS journey - by the end of it your game will run on a *single server-worker* on SpatialOS. You will be ready to start adding multiserver logic to take advantage of the distributed architecture of SpatialOS.

<%(Callout type="alert" message="The GDK's porting workflow is currently in pre-alpha as we improve its stability. We do not recommend attempting to port your Unreal game now. If you need to port your game, please get in touch on our [forums](https://forums.improbable.io/), or on [Discord](https://discord.gg/vAT7RSU) so we can best support you. We intend have a stable porting workflow in Q1 2019. Thanks for your patience.")%>

## Before you start

Before porting your project: 
 
* If you haven't done this already, install SpatialOS and the GDK's dependencies and clone the SpatialOS Unreal Engine fork by following:
    * [Getting started: 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
    * [Getting started: 2 - Get and build the GDK’s Unreal Engine Fork]({{urlRoot}}/content/get-started/build-unreal-fork)
* If you haven't done this already, clone the GDK repository by following [our SpatialOS GDK Starter template guide]({{urlRoot}}/content/get-started/gdk-template). Later in this guide, you will copy some necessary files from the template project over to yours.

* Make sure your Spatial CLI is up to date: from a terminal window, run the command `spatial update`. 
<!-- // TODO: Update the set up link when ready -->

### Terms used in this guide
`<GameRoot>` - The folder containing your project's `.uproject` file and `Source` folder.  
`<ProjectRoot>` - The folder containing your `<GameRoot>`.  
`<YourProject>` - Name of your game project's `.uproject` (for example, `\<GameRoot>\StarterProject.uproject`).

## Port your game to the GDK

<%(TOC)%>

### 1. Set up the project structure
1. Ensure you have a `<ProjectRoot>`. If your `<GameRoot>` lives inside of a self-contained folder already, this is your `<ProjectRoot>`. If not, you should create a new folder to represent your `<ProjectRoot>` and move your `<GameRoot>` inside of it.  
   
    Your project structure should take the form of `\<ProjectRoot>\<GameRoot>\<YourProject>.uproject`
    For example:
    `\StarterProject\Game\StarterProject.uproject`
    
    > This step is essential as the `spatial` folder must be located in the directory above your `<GameRoot>`. This is so that the GDK scripts work correctly with Unreal.
1. Your game's project needs some extra files and folders to run with the GDK; you can copy these from the template project that you set up earlier in the [Before you start](#before-you-start) section.

    To do this: either in a terminal window or your file manager, navigate to the root of the `StarterProject` repository and copy all of the files and folders below to your `<ProjectRoot>`:  

    ``` cpp
    \StarterProject\spatial\
    \StarterProject\LaunchClient.bat 
    \StarterProject\LaunchServer.bat
    \StarterProject\ProjectPaths.bat
    ```
    Your game's folder structure should now resemble:

    ``` cpp
    \<ProjectRoot>\<GameRoot>\
    \<ProjectRoot>\spatial\
    \<ProjectRoot>\LaunchClient.bat 
    \<ProjectRoot>\LaunchServer.bat
    \<ProjectRoot>\ProjectPaths.bat
    etc...
    ```


1. Our helper scripts require configuration to work correctly. Set up your project paths:  
   Open **`\<ProjectRoot>\ProjectPaths.bat`** for editing and:  

    * In `set PROJECT_PATH=Game`, replace `Game` with your `<GameRoot>` folder name.  
    * In `set GAME_NAME=StarterProject`, replace `StarterProject` with the name of your game's `.uproject` (`<YourProject>` [terms used in this guide](#terms-used-in-this-guide)).  
    
    > Doing this incorrectly will result in the helper scripts `LaunchClient.bat` and `LaunchServer.bat` not working and printing that the path specified does not exist when trying to use them.

1. Run `Setup.bat` which is in the root directory of the GDK repository you cloned (this should be `<ProjectRoot>\<GameRoot>\Plugins\UnrealGDK\`). To do this either:
    - In a terminal window, navigate to the root directory of the GDK and run: `Setup.bat` or
    - In your file manager, double-click the file.
  
    > Note: This requires authorization with your SpatialOS account via a web browser. `Setup.bat` will launch the authorization page.

### 2. Add the SpatialGDK module to your project
1. In your project's `*.build.cs` file, add `"SpatialGDK"` to the `PublicDependencyModuleNames`.
    For example:  

    ``` csharp
    PublicDependencyModuleNames.AddRange(`
                new string[] {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "OnlineSubsystem",
                    "OnlineSubsystemUtils",
                    "AssetRegistry",
                    "AIModule",
                    "GameplayTasks",
                    "SpatialGDK",
                }
            );
    ```

### 3. Build your project
Set up your Unreal game project to work with the GDK for Unreal fork of the Unreal Engine, which you cloned and installed in the [Before you start](#before-you-start) section. To do this:

1. In File Explorer, navigate to `<ProjectRoot>\<GameRoot>`.
1. Right-click your `.uproject` file and select **Switch Unreal Engine version**.
1. Select the path to the Unreal Engine fork you cloned earlier.
1. In File Explorer, right-click your `.uproject` file and select **Generate Visual Studio project files**.
1. Open the generated solution file in Visual Studio and in the Development Editor build configuration, compile and run the project.

### 4. Modify Unreal classes for GDK compatibility
It is necessary to modify your `GameInstance` class to work properly with the GDK.  

1. Make your `GameInstance` inherit from `SpatialGameInstance`.  <br/>

   > If you have not yet made a `GameInstance` for your game and are still using the default, you must either create a Blueprint or a native `GameInstance` class now. Remember to configure your `Project Settings` to use this new `GameInstance` as default, under **Project Settings > Project Maps and Modes > Game Instance > Game Instance Class**. <br/>


* If your game's `GameInstance` is a C++ class, locate its header file and add the following `#include`:
    `"SpatialGameInstance.h"`

    For example:
    ``` cpp
    #include "CoreMinimal.h"
    #include "SpatialGameInstance.h"
    #include "YourProjectGameInstance.generated.h"
    ```
    Then, under `UCLASS()`, change the parent class from `UGameInstance` to `USpatialGameInstance`:

    For example:  

    ```cpp
    UCLASS()
    class YOURPROJECT_API UYourProjectGameInstance : public USpatialGameInstance
    {
        GENERATED_BODY()
    };
    ```

* If your `GameInstance` is a Blueprint class, you need to open and edit it in the Blueprint Editor: from the Blueprint Editor toolbar, navigate to the **Class Settings**. In **Class Options** set the **Parent Class** to `SpatialGameInstance`

    ![spatial game instance reparent]({{assetRoot}}assets/screen-grabs/spatial-game-instance-reparent.png)


### 5. Add GDK configurations
The steps below reference and introduce the following SpatialOS terms: [workers]({{urlRoot}}/content/glossary#workers), [schema]({{urlRoot}}/content/glossary#schema), [Schema Generation]({{urlRoot}}/content/glossary#schema-generation) [SpatialOS components]({{urlRoot}}/content/glossary#spatialos-component), [checking out]({{urlRoot}}/content/glossary#check-out), [streaming queries]({{urlRoot}}/content/glossary#streaming-queries), [Singleton Actor]({{urlRoot}}/content/glossary#singleton-actor), [deployment]({{urlRoot}}/content/glossary#deployment), [launch configuration]({{urlRoot}}/content/glossary#launch-configuration), [snapshot]({{urlRoot}}/content/glossary#snapshot), [worker configuration]({{urlRoot}}/content/glossary#worker-configuration).
<br/>You can find out about them in the [glossary]({{urlRoot}}/content/glossary) but you don't need to know about them in detail to complete the port.

1. In `<GameRoot>\Config`, open `DefaultEngine.ini` and add:

    ``` ini
    [/Script/Engine.Engine]
    !NetDriverDefinitions=ClearArray
    +NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SpatialGDK.SpatialNetDriver",DriverClassNameFallback="/Script/SpatialGDK.SpatialNetDriver")

    [/Script/SpatialGDK.SpatialNetDriver]
    NetConnectionClassName="/Script/SpatialGDK.SpatialNetConnection"
    ```

    > These lines ensure that the GDK can override Unreal's network replication.
    >
    > If you ever need to run with default Unreal networking for workflow or validation reasons, you can switch networking in the Unreal Editor: from the Editor menu, click the down arrow on the **Play** button and from the drop-down menu un-check `Spatial Networking`. This setting is valid for Editor and command-line builds. It is stored in your game project's Unreal config file; `<GameRoot>\Config\DefaultGame.ini` under `/Script/EngineSettings.GeneralProjectSettings`.
    >
    > **Warning:** As the GDK is in alpha, switching back to Unreal default networking mode can be a useful way to check for any divergence between GDK behavior and the default Unreal behavior. This can be helpful in isolating the root cause of issues you see while debugging. However, you lose access to the multiserver features of the GDK in Unreal default networking mode which may lead to erratic behavior.

### 6. Generate schema and a snapshot
You need to generate [schema]({{urlRoot}}/content/glossary#schema) and generate a [snapshot]({{urlRoot}}/content/glossary#snapshot) to get your game's deployment started. To do this:

1. In the Unreal Editor, on the [GDK toolbar]({{urlRoot}}/content/toolbars), click the **Schema** button to run the [Schema Generator]({{urlRoot}}/content/glossary#schema-generation).
1. On the same toolbar, click the **Snapshot** button which will generate a snapshot for the map currently open in the editor.

    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbars-basic.png)

### 7. Launch your game
1. Switch your game project to use the SpatialOS networking. To do this: in the Unreal Editor, from the toolbar, open the **Play** drop-down menu and check two checkboxes:
    * Check the box for **Run Dedicated Server**
    * Check the box for **Spatial Networking**

    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-checkboxes.png)

    > From this drop-down menu it is possible to increase the number of servers that will be launched. For now leave this at 1. This is because there is currently no multiserver logic in your code. This port will be the baseline for you to start building the multiserver game logic.  
1. Still in the Unreal Editor but this time from the [GDK toolbar]({{urlRoot}}/content/toolbars), select the green **Start** button. This builds your [worker configuration]({{urlRoot}}/content/glossary#worker-configuration) file and launches your game in a local deployment. <br/>
**Start** opens up a terminal window and runs two SpatialOS command line interface ([CLI]({{urlRoot}}/content/glossary#spatial-command-line-tool-cli) commands: `spatial build build-config` and `spatial local launch`. It is finished when you see `SpatialOS ready` in the terminal window.
1. On the main Unreal toolbar, click **Play**. 
1. From the SpatialOS [GDK toolbar]({{urlRoot}}/content/toolbars) click **Inspector** which will open a local [SpatialOS inspector](https://docs.improbable.io/reference/latest/shared/operate/inspector) in your web browser. Here you can see the entities and their components present in your deployment, updates are in real-time.

    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbars-basic.png)

**For running a local deployment with managed workers or a cloud deployment take a look at the [glossary section for deployments]({{urlRoot}}/content/glossary#deployment)**

**Job done!** You have ported your Unreal game to run on SpatialOS. Move around and look at the changes reflected in your inspector.

If you have encountered any problems please check out our [troubleshooting]({{urlRoot}}/content/troubleshooting) and [known-issues]({{urlRoot}}/known-issues).

#### Logs
You can find Spatial log files for your local deployments in `<ProjectRoot>\spatial\logs\`.  

* `spatial_<datetime>.log` contains all of the logs printed to your terminal during the local deployment.  
* There are also timestamped folders here which contain additional logs:
  1. `<ProjectRoot>\spatial\logs\workers\` contain managed worker logs which are the workers started by SpatialOS, specified in your [launch configuration]({{urlRoot}}/content/glossary#launch-configuration).
  1. `<ProjectRoot>\spatial\logs\runtime.log` contains the logs printed by the SpatialOS runtime. These are the services required for SpatialOS to run a local deployment.  

If you require additional debugging logs you can always run `spatial local launch` with the flag `--log_level=debug`.

#### How to modify the default behavior
You can modify some of the GDK settings from the Unreal Editor toolbar at **Edit** > **Project Settings** >**SpatialOS Unreal GDK** > **Toolbar**.
You can change:

* the snapshot file's filename and location
* the launch configuration

## Next steps
You can now begin experimenting with the multiserver features offered by the GDK.

If you haven't already, check out the tutorial on how to implement [cross-server shooting]({{urlRoot}}/content/get-started/tutorial).  
Also check out the documentation on [cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs), [handover]({{urlRoot}}/content/handover-between-server-workers) and [Singleton Actors]({{urlRoot}}/content/singleton-actors).
