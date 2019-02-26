# Get started: Port your own Unreal project to the GDK

<%(TOC)%>

As an experienced UE4 developer, you likely have a prototype or a game already. The GDK for Unreal allows you to port you game over to SpatialOS. 

This guide shows you how to kickstart your SpatialOS journey - by the end of it your game will run on a *single server-worker* on SpatialOS and you will be ready to start adding multiserver logic to take advantage of the distributed architecture of SpatialOS.

<%(Callout type="alert" message="The GDK's porting workflow is currently in pre-alpha as we improve its stability. We do not recommend attempting to port your Unreal game now. If you need to port your game, please get in touch on our [forums](https://forums.improbable.io/), or on [Discord](https://discord.gg/vAT7RSU) so we can best support you. We intend have a stable porting workflow in Q1 2019. Thanks for your patience.")%>

## Before you start

Before porting your project: 
 
* If you haven't done this already, install SpatialOS and the GDK's dependencies and clone the SpatialOS Unreal Engine fork by following:
    * [Getting started: 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
    * [Getting started: 2 - Get and build the GDK’s Unreal Engine Fork]({{urlRoot}}/content/get-started/build-unreal-fork)
* If you haven't done this already, clone the GDK repository by following [our SpatialOS GDK Starter template guide]({{urlRoot}}/content/get-started/gdk-template). Later in this guide, you will copy some necessary files from the template project over to yours.

* Open a terminal window and run the command `spatial update` to ensure your Spatial CLI installation is up to date. 

### Terms used in this guide
`<GameRoot>` - The folder containing your project's `.uproject` file and `Source` folder.  
`<ProjectRoot>` - The folder containing your `<GameRoot>`.  
`<YourProject>` - Name of your game project's `.uproject` (for example, `\<GameRoot>\StarterProject.uproject`).

## Port your game to the GDK

### 1. Set up the project structure


1. Create a new directory to represent your `<ProjectRoot>` and move your `<GameRoot>` directory inside of it.  
   
    Your project structure should be:  `\<ProjectRoot>\<GameRoot>\<YourProject>.uproject`<br/>
    
    For example:
    `\StarterProject\Game\StarterProject.uproject`
    
    **Note**: This step is essential as the `spatial` folder must be located in the directory above your `<GameRoot>`.
1. Your project needs some extra files and folders to run with the GDK; you can copy these from the template project that you set up earlier in the [Before you start](#before-you-start) section.

    To do this: either in a terminal window or your file manager, navigate to the root of the `StarterProject` repository and copy all of the files and directories below to your `<ProjectRoot>`:  

    ``` cpp
    \StarterProject\spatial\
    \StarterProject\LaunchClient.bat 
    \StarterProject\LaunchServer.bat
    \StarterProject\ProjectPaths.bat
    ```
    Your project's directory structure should now resemble:

    ``` cpp
    \<ProjectRoot>\<GameRoot>\
    \<ProjectRoot>\spatial\
    \<ProjectRoot>\LaunchClient.bat 
    \<ProjectRoot>\LaunchServer.bat
    \<ProjectRoot>\ProjectPaths.bat
    etc...
    ```

1. You need to configure the GDK helper scripts so they work correctly. 

Follow this step to set up your project paths: 
   
   * Open **`\<ProjectRoot>\ProjectPaths.bat`** in a text editor.  

    * In `set PROJECT_PATH=Game`, replace `Game` with your `<GameRoot>` folder name.  
    * In `set GAME_NAME=StarterProject`, replace `StarterProject` with the name of your game's `.uproject` (`<YourProject>` [terms used in this guide](#terms-used-in-this-guide)).  
    
**Note**: The helper scripts `LaunchClient.bat` and `LaunchServer.bat` will not work if you do not follow this step correctly. 

1. Run `Setup.bat` which is in the root directory of the GDK repository you cloned (this should be `<ProjectRoot>\<GameRoot>\Plugins\UnrealGDK\`). To do this either:
    - In a terminal window, navigate to the root directory of the GDK and run: `Setup.bat` or
    - In your file manager, double-click the `Setup.bat` file.
  
    **Note**:`Setup.bat` will automatically open the SpatialOS authorization page in your default browser. You may be prompted to sign into your SpatialOS account if you have not already. 

### 2. Add the SpatialGDK module to your project

1. In File Explorer, navigate to `\<ProjectRoot>\<GameRoot>\Source\<YourProject>\`.
2. Open the `<YourProject>.build.cs` file in a code editor and add add `"SpatialGDK"` to `PublicDependencyModuleNames`.
    
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
Set up your Unreal project to work with the GDK for Unreal fork of the Unreal Engine, which you cloned and installed in the [Before you start](#before-you-start) section. To do this:

1. In File Explorer, navigate to `<ProjectRoot>\<GameRoot>`.
1. Right-click your `.uproject` file and select **Switch Unreal Engine version**.
1. Select the path to the Unreal Engine fork you cloned earlier.
1. In **File Explorer**, right-click `<YourProject>`.uproject and select Generate Visual Studio Project files.
1. In the same directory, double-click **`<YourProject>`.sln** to open it with Visual Studio.
1. On the Visual Studio toolbar, set your Solution configuration to **Development Editor**. <br/>
    ![our SpatialOS GDK Starter template guide]({{AssetRoot}}/porting-solution-config.png)<br/>
    _Image: The Visual Studio toolbar, with the Development Editor Solution configuration highlighted in red._
1. In the Solution Explorer window, right-click on **`<YourProject>`** and select **Build**.


### 4. Modify Unreal classes for GDK compatibility
You must modify your `GameInstance` class to work properly with the GDK.  

1. Make your `GameInstance` inherit from `SpatialGameInstance`.  <br/>

   > If you have not yet made a `GameInstance` for your game and are still using the default `GameInstance`, you must either create a Blueprint or a native `GameInstance` class now. Remember to configure your `Project Settings` to use this new `GameInstance` by default, under **Project Settings > Project Maps and Modes > Game Instance > Game Instance Class**. <br/>


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

### 5. Generate schema and a snapshot
You need to generate [schema]({{urlRoot}}/content/glossary#schema) and generate a [snapshot]({{urlRoot}}/content/glossary#snapshot) to get your game's deployment started. To do this:

1. In the Unreal Editor, on the [GDK toolbar]({{urlRoot}}/content/toolbars), click the **Schema** button to run the [Schema Generator]({{urlRoot}}/content/glossary#schema-generation).
1. On the same toolbar, click the **Snapshot** button which will generate a snapshot for the map currently open in the editor.

    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbars-basic.png)

### 6. Launch your game
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

You have now ported your Unreal game to run on SpatialOS. Move around and look at the changes reflected in your inspector.

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
