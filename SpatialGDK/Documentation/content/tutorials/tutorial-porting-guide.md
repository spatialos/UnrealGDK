<%(TOC)%>
# Tutorials and guides
## Port your project to SpatialOS

<%(Callout type="warn" message="Please be aware of the [GDK's support level of different Unreal features]({{urlRoot}}/unreal-features-support). If you need to port your game, please contact us via our [forums](https://forums.improbable.io/), or [Discord](https://discord.gg/vAT7RSU) so we can best support you.")%>

This guide shows you how to port your own Unreal project to the GDK. By the end of this guide, your game will run on a single [server-worker]({{urlRoot}}/content/glossary#workers) on SpatialOS.


**Get to know the GDK before porting your game**</br>
We recommend following steps 1 to 3 of the [Get started]({{urlRoot}}/content/get-started/introduction) guide and setting up the [Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) before porting your project. This gives you an overview of the GDK and using SpatialOS.
<br/>

**Tip: Reference project** 
<br/>
 As you port your own Unreal project to SpatialOS, you could use our pre-ported [Unreal Shooter Game](https://docs.unrealengine.com/en-us/Resources/SampleGames/ShooterGame) as a reference. You should already have this project as it is included in the `Samples` directory of [the SpatialOS Unreal Engine fork](https://github.com/improbableio/UnrealEngine) which you downloaded as part of the _Get Started_ steps. </br>
(If you want to see the game running, there's a [video on youtube](https://www.youtube.com/watch?v=xojgH7hJgQs&feature=youtu.be) to check out.) 

**Terms used in this guide**</br>
* `<GameRoot>` - The directory containing your project's `.uproject` file and `Source` directory.  
* `<ProjectRoot>` - The directory containing your `<GameRoot>`.  
* `<YourProject>` - The name of your project's `.uproject` file (for example, `\<GameRoot>\TP_SpatialGDK.uproject`).


## 1. Before you start

Before porting your project: 

* Set up the GDK and Starter Template.</br>
If you haven't done this already, follow the three steps in _Get started_ guide in order to port your game:
    1. [Get started 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
    1. [Get started 2 - Get and build the GDK’s Unreal Engine Fork]({{urlRoot}}/content/get-started/build-unreal-fork)
    1. [Get started 3 - Set up a project: The Starter Template]({{urlRoot}}/content/get-started/gdk-template) </br>
    (Note that you must follow the Starter Template instructions and not the Example Project instructions.)
</br>
</br>
* Update your local version of the spatial CLI. s</br>
Open a terminal window and run the command `spatial update` to ensure your [spatial CLI]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli) installation is up to date. 
</br>
</br>


## 2. Set up the project structure


1. Create a new empty directory to represent your `<ProjectRoot>` and move your `<GameRoot>` directory inside of it.  
   
    Your project structure should be:  `\<ProjectRoot>\<GameRoot>\<YourProject>.uproject`<br/>
    
    For example:
    `\MyProject\Game\TP_SpatialGDK.uproject`
    
2. Your project needs some extra files and folders to run with the GDK. Copy these files from the template project that you set up earlier in the [Before you start](#1-before-you-start) section.

    To do this: either in a terminal window or your file manager, navigate to the root of the `StarterTemplate` repository and copy all of the files and directories below to your `<ProjectRoot>`:  

    ``` cpp
    \TP_SpatialGDK\spatial\
    \TP_SpatialGDK\LaunchClient.bat 
    \TP_SpatialGDK\LaunchServer.bat
    \TP_SpatialGDK\ProjectPaths.bat
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
    
    **Note**: You must place the `spatial` directory in the directory above your `<GameRoot>`.
<br/><br/>

1. You need to configure the GDK helper scripts to work with your project. 

Follow this step to set up your project paths: 

   * Open **`\<ProjectRoot>\ProjectPaths.bat`** in a text editor.  

        * In `set PROJECT_PATH=Game`, replace `Game` with your `<GameRoot>` folder name.  
        * In `set GAME_NAME= TP_SpatialGDK `, replace `TP_SpatialGDK ` with the name of your game's `.uproject` (`<YourProject>` [terms used in this guide](#terms-used-in-this-guide)).  

    **Note**: The helper scripts `LaunchClient.bat` and `LaunchServer.bat` will not work if you do not follow this step correctly. 

## 3. Clone the GDK

Now you need to clone the SpatialOS GDK for Unreal into your project. To do this: 

1. In **File Explorer**, navigate to the `<GameRoot>` directory and create a `Plugins` folder in this directory.
2. In a Git Bash terminal window, navigate to `<GameRoot>\Plugins` and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDK.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDK.git`<br/><br/>
**Note:** You need to ensure that the root directory of the GDK for Unreal repository is called `UnrealGDK` so the file path is: `<GameRoot>\Plugins\UnrealGDK\...`<br/>
3. Run `Setup.bat` which is in the root directory of the GDK repository (this should be `<ProjectRoot>\<GameRoot>\Plugins\UnrealGDK\`). To do this either:
    - In a terminal window, navigate to the root directory of the GDK and run: `Setup.bat` or
    - In your file manager, double-click the `Setup.bat` file.
      

    **Note**:`Setup.bat` will automatically open the SpatialOS authorization page in your default browser. You may be prompted to sign into your SpatialOS account if you have not already. 

## 4. Add the SpatialGDK module to your project

1. In **File Explorer**, navigate to `\<ProjectRoot>\<GameRoot>\Source\<YourProject>\`.
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

## 5. Build your project
Set up your Unreal project to work with the GDK for Unreal fork of the Unreal Engine, which you cloned and installed in the [Before you start](#1-before-you-start) section. To do this:

1. In **File Explorer**, navigate to `<ProjectRoot>\<GameRoot>`.
1. Right-click your `<YourProject>.uproject` file and select **Switch Unreal Engine version**.
1. Select the path to the Unreal Engine fork you cloned earlier.
1. In **File Explorer**, right-click `<YourProject>`.uproject and select **Generate Visual Studio Project files**. This automatically generates a Visual Studio solution file for your project called `<YourProject.sln>`
1. In the same directory, double-click `<YourProject>`.sln to open it with Visual Studio.
1. On the Visual Studio toolbar, set your Solution configuration to **Development Editor**. <br/>
    ![Visual studio toolbar]({{assetRoot}}assets/screen-grabs/porting-solution-config.png)<br/>
    _Image: The Visual Studio toolbar, with the Development Editor Solution configuration highlighted in red._
1. In the Solution Explorer window, right-click on **`<YourProject>`** and select **Build**.


## 6. Modify Unreal classes for GDK compatibility
You must modify your `GameInstance` class to work properly with the GDK.  

1. Make your `GameInstance` inherit from `SpatialGameInstance`.  <br/>

   > If you have not made a `GameInstance` for your game and are still using the default `GameInstance`, you must either create a Blueprint or a native `GameInstance` class now. Remember to configure your `Project Settings` to use this new `GameInstance` by default, under **Project Settings > Project Maps and Modes > Game Instance > Game Instance Class**. <br/>

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

* If your `GameInstance` is a Blueprint class, you need to open and edit it in the Blueprint Editor: 
        * From the Blueprint Editor toolbar, navigate to the **Class Settings**. In **Class Options** set the **Parent Class** to `SpatialGameInstance`.

    ![spatial game instance reparent]({{assetRoot}}assets/screen-grabs/spatial-game-instance-reparent.png)<br/>_Image: The Blueprint class settings screen_<br/>

## 7. Generate schema and a snapshot
You need to generate [schema]({{urlRoot}}/content/spatialos-concepts/schema-and-snapshots#schema) and generate a [snapshot]({{urlRoot}}/content/how-to-use-snapshots) before you start your deployment. To do this:

1. In the Unreal Editor, on the GDK toolbar, open the **Schema** drop-down menu and select **Schema (Full Scan)**. <br/>
       ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button-full-scan.png)<br/>
     _Image: On the GDK toolbar in the Unreal Editor, select **Schema (Full Scan)**_<br/>
1. Select **Snapshot** to generate a snapshot.<br/>
    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)<br/>
    _Image: On the GDK toolbar in the Unreal Editor, select **Snapshot**_<br/>

## 8. Launch your game
1. Switch your game project to use the SpatialOS networking. To do this: 

* In the Unreal Editor, from the toolbar, open the **Play** drop-down menu and check two checkboxes:
    * Check the box for **Run Dedicated Server**
    * Check the box for **Spatial Networking**

    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/toolbar-checkboxes.png)<br/>
    _Image: The Unreal Engine launch settings drop-down menu_<br/><br/>

    > You can increase the number of servers that you launch by changing the **Number of servers** value. Leave this value at 1 for now. This is because there is currently no multiserver logic in your code. After you have completed this guide you can start building multiserver game logic.  
    
1. On the [GDK toolbar]({{urlRoot}}/content/toolbars), select **Start**. This builds your [worker configuration]({{urlRoot}}/content/glossary#worker-configuration) file and launches your game in a local deployment. <br/>     
![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/start-button.png)<br/>
_Image: On the GDK toolbar in the Unreal Editor select **Start**_<br/><br/>
Selecting **Start** opens a terminal window and runs two SpatialOS command line interface ([CLI]({{urlRoot}}/content/glossary#spatialos-command-line-tool-cli) commands: `spatial build build-config` and `spatial local launch`. Your deployment has started when you see `SpatialOS ready` in the terminal window.<br/><br/>
1. On the main Unreal toolbar, select **Play**. <br/>![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/play-button.png)<br/>
_Image: On the Unreal Engine toolbar **Play**_<br/><br/>
1. From the SpatialOS [GDK toolbar]({{urlRoot}}/content/toolbars) select **Inspector**, which opens a local [SpatialOS Inspector](https://docs.improbable.io/reference/latest/shared/operate/inspector) in your default web browser. Here you can see the entities and their components present in your deployment.<br/>![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/inspector-button.png)<br/>
_Image: On the GDK toolbar in the Unreal Editor select **Inspector**_<br/>

**For running a local deployment with managed workers or a cloud deployment take a look at the [glossary section for deployments]({{urlRoot}}/content/glossary#deployment)**

You have now ported your Unreal game to run on SpatialOS. Move around and look at the changes reflected in the inspector.

If you have encountered any problems please check out our [troubleshooting]({{urlRoot}}/content/troubleshooting) and [known-issues]({{urlRoot}}/known-issues).

</br>
### Logs
You can find Spatial log files for your local deployments in `<ProjectRoot>\spatial\logs\`.  


* `spatial_<datetime>.log` contains all of the logs printed to your terminal during the local deployment.  
* There are also timestamped folders here which contain additional logs:
  1. `<ProjectRoot>\spatial\logs\workers\` contain managed worker logs which are the workers started by SpatialOS, specified in your [launch configuration]({{urlRoot}}/content/glossary#launch-configuration).
  1. `<ProjectRoot>\spatial\logs\runtime.log` contains the logs printed by the SpatialOS Runtime. These are the services required for SpatialOS to run a local deployment.  

If you require additional debugging logs you can run `spatial local launch` with the flag `--log_level=debug`.

</br>
### Modify the default behavior
You can modify some of the GDK settings from the Unreal Editor toolbar at **Edit** > **Project Settings** >**SpatialOS Unreal GDK** > **Toolbar**.
You can change:

* the snapshot file's filename and location
* the launch configuration

</br>
**Next steps:** </br>
If you haven't already, check out:

* The tutorial on [multiple deployments for session-based games]({{urlRoot}}/content/tutorials/deployment-manager/tutorial-deploymentmgr-intro).
*  The Multiserver Shooter tutorial to learn how to implement [cross-server interactions]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro).  

Also check out the documentation on [cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs), [handover]({{urlRoot}}/content/actor-handover) and [Singleton Actors]({{urlRoot}}/content/singleton-actors).




<br/>------</br>
_2019-04-11 Added Shooter Game as a reference project with limited editorial review._
