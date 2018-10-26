# Port your Unreal project to the GDK

## Before you start

Before porting your project, you need to install and set up the SpatialOS GDK: follow the [set up guide]({{urlRoot}}/setup-and-installing) to the end of **Setting up the Unreal GDK module and Starter Project** > **Cloning**.
<!-- // TODO: Update the set up link when ready -->

### Terms used in this guide
`<ProjectRoot>` - The root folder of your Unreal project.  
`<GameRoot>` - The folder containing your game project's `.uproject` and source folder (for example, `<ProjectRoot>/ShooterGame/`).  
`<YourProject>` - Name of your game project's `.uproject` (for example, `StarterProject.uproject`).

## Start porting!

<%(TOC)%>

### 1. Set up the project structure
1. Run Setup.bat which is in the root directory of the GDK repository you cloned (this should be `<ProjectRoot>\<GameRoot>\Plugins\UnrealGDK\`). To do this either:
    - In a terminal window, navigate to the root directory of the GDK and run: `Setup.bat` or
    - In your file manager, double-click the file.
    > Note: This requires authorization with your SpatialOS account via a web browser. `Setup.bat` will launch the authorization page.
1. Your project needs some extra files and folders to run with the GDK; you can copy these from the StarterProject repository that you cloned earlier.

    To do this: either in a terminal window or your file manager, navigate to the root of the `StarterProject` repository and copy all of the files and folders except the `\Game\` _folder_ into your `<ProjectRoot>` folder. Your game's folder structure should now resemble:
    
        ``` cpp
        \<ProjectRoot>\.git\
        \<ProjectRoot>\<GameRoot>\
        \<ProjectRoot>\spatial\
        \<ProjectRoot>\.gitignore
        \<ProjectRoot>\LaunchClient.bat 
        \<ProjectRoot>\LaunchServer.bat
        \<ProjectRoot>\LICENSE.md
        \<ProjectRoot>\ProjectPaths.bat
        \<ProjectRoot>\README.md
        etc...
        ```
1. Our helper scripts require configuration to work correctly. Set up your project paths:
   
   Open **`\<ProjectRoot>\ProjectPaths.bat`** for editing and:
    - In `set PROJECT_PATH=Game`, replace `Game` with your `<GameRoot>` folder name.
    - In `set GAME_NAME=StarterProject`, replace `StarterProject` with the name of your game's `.uproject` (which we'll refer to as `<YourProject>`).

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

### 3. Modify Unreal classes for GDK compatibility
<!-- // TODO: Handle this for blueprint projects -->
1. In your project's `GameInstance` header file(s), add the following `#include`:
`"SpatialGameInstance.h"`

    For example:
    ``` cpp
    #include "CoreMinimal.h"
    #include "SpatialGameInstance.h"
    #include "YourProjectGameInstance.generated.h"
    ```
    Then, under `UCLASS()`, change `UGameInstance` to `USpatialGameInstance`:

    For example:
    ```cpp
    UCLASS()
    class YOURPROJECT_API UYourProjectGameInstance : public USpatialGameInstance
    {
    GENERATED_BODY()
    };
    ```

1. In your game's `PlayerController` header file(s), add the public method declaration
`virtual void InitPlayerState() override;`.

1. In your game's `PlayerController` class(es), add the following definition of the `InitPlayerState` function:

    ``` cpp
    void AYourProjectPlayerController::InitPlayerState()
    {
        UWorld* World = GetWorld();
        check(World);
        USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
        if (NetDriver)
        {
            const FEntityId EntityId = NetDriver->GetEntityRegistry()->GetEntityIdFromActor(this);
            UE_LOG(LogTemp, Log, TEXT("PC:InitPlayerState called with entity id %d"), EntityId.ToSpatialEntityId());
            if (EntityId != 0)
            {
                // EntityId is not 0, which means that this PC has already been initialized.
                return;
            }
        }

        Super::InitPlayerState();
    }
    ```

1. In the file(s) where you added the `InitPlayerState` function definition, add the following `#include`s:

    ``` cpp
    #include "SpatialNetDriver.h"
    #include "EntityId.h"
    #include "EntityRegistry.h"
    ```

### 4. Add GDK configurations
The steps below reference the introduce the following SpatialOS terms: [workers]({{URLRoot}}/content/glossary#workers), [schema]({URLRoot}}/content/glossary#schema), [Schema Generator]({{URLRoot}}/content/glossary#schema-generator) [SpatialOS components]({{URLRoot}}/content/glossary#spatialos-component), [checking out]({{URLRoot}}/content/glossary#check-out), [streaming queries]({{URLRoot}}/content/glossary#streaming-queries), [Singleton Actor]({{URLRoot}}/content/glossary#singleton-actor), [deployment]({{URLRoot}}/content/glossary#deployment), [launch configuration]({{URLRoot}}/content/glossary#launch-configuration), [snapshot]({{URLRoot}}/content/glossary#snapshot), [worker configuration]({{URLRoot/content/glossary#worker-configuration).
<br/>You can find out about them in the [glossary]({URLRoot}}/content/glossary) but you don't need to know about them in detail to complete the port.
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
    > If you ever need to run with default Unreal networking for workflow or validation reasons, you can switch networking in the Unreal Editor: from the Editor menu, select **Play** and from the drop-down menu un-check `Spatial Networking`. This setting is valid for Editor and command-line builds. It is stored in your game project's Unreal config file; `<GameRoot>\Config\DefaultGame.ini` under `\Script\EngineSettings.GeneralProjectSettings`.
    >
    > **Warning:** As the GDK is in alpha, switching back to Unreal default networking mode can be a useful way to debug and so speed up your development iteration. However, you lose access to the multi-server features of the GDK in Unreal default networking mode which may lead to erratic behavior.

<!-- // TODO: Handle this part below for projects which use default GameMode and GameState -->
1. Specify Singleton Actors  
The GDK uses [Singleton Actors]({{urlRoot}}/content/singleton-actors) - these are server-side authoritative Actors which are a single source of truth for both operations and data across a multi-server simulation; `GameState` and `GameMode` are examples of this. To tell SpatialOS how to replicate Singleton Actors, the GDK  has a `UCLASS` [specifier (Unreal docs)](https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/Reference/Classes/Specifiers). You add this to classes you want to be Singleton Actors.  
   
    1. If your game's `GameState` is a `.cpp` file, locate it and mark it as a Public Singleton by modifying the `UCLASS` specifier as shown:
        ```
        UCLASS(SpatialType=Singleton)
        ```
        If your game's `GameState` is a Blueprint, you need to open and edit it in the Blueprint Editor: from the Blueprint Editor toolbar, navigate to the **Class Settings**. In **Class Options** check **Spatial Type** and in the Spatial Description text box enter `Singleton`.

    1. If your game's `GameMode` is a `.cpp` file, locate it and mark it as a Private Singleton by modifying the `UCLASS` specifier as shown:
        ```
        UCLASS(SpatialType=(Singleton,ServerOnly))
        ```
       If your game's `GameMode` is a Blueprint, you need to open and edit it in the Blueprint Editor: from the Blueprint Editor toolbar, navigate to the **Class Settings**. In **Class Options** check **Spatial Type** and in the **Spatial Description** text box enter `Singleton,ServerOnly`.

   Marking these Singleton Actor classes as `Spatial Type` enables them to work with SpatialOS as [schema]({URLRoot}}/content/glossary#schema) will now be generated for them. The generation of schema will be handled in [Step 6](6)
    <!-- // TODO: Step 6 relative link  -->

### 5. Generate code for SpatialOS C++ workers and build your project
Set up your Unreal game project to work with the GDK for Unreal fork of the Unreal Engine, which you cloned and installed in the [Before you start]({{URLRoot}}/content/porting-unreal-project-to-gdk#Before-you-start) section. To do this:
<!-- // TODO: before you start relative link  -->
1. In File Explorer, navigate to `<ProjectRoot>\<GameRoot>`.
1. Right-click your `.uproject` file and select **Switch Unreal Engine version**.
1. Select the path to the Unreal Engine fork you cloned earlier.
1. Build workers for a local [deployment]({{URLRoot}}/content/glossary#deployment) of your game. To do this:  
   
    In a terminal window, from `<ProjectRoot>\<GameRoot>\Plugins\UnrealGDK\SpatialGDK\Build\Scripts`, run `BuildWorker.bat <YourProject>Editor Win64 Development <YourProject>.uproject`  

    If, for example, your project is called "StarterProject", this command would be: `BuildWorker.bat StarterProjectEditor Win64 Development StarterProject.uproject`.  
    This means you can now run a local deployment with more than one server; your [launch configuration]({{URLRoot}}/content/glossary#launch-configuration) can have multiple servers-workers.
1. In File Explorer, right-click your `.uproject` file and select **Generate Visual Studio project files**.
1. Open the generated solution file in Visual Studio and in the Development Editor build configuration, compile and run the project.

### 6. Generate schema and a snapshot
You need to generate [schema]({URLRoot}}/content/glossary#schema) and generate a [snapshot]({{URLRoot}}/content/glossary#snapshot) to get your game's deployment started. To do this:
1. In the Unreal Editor, on the GDK toolbar, click  the **Schema** button to run the [Schema Generator]({{URLRoot}}/content/glossary#schema-generator).
1. On the same toolbar, click the **Snapshot** button which will generate a snapshot for the map currently open in the editor.

### 7. Launch your game
1. Switch your game project to use the SpatialOS networking. To do this: in the Unreal Editor, from the toolbar, open the **Play** drop-down menu and check two checkboxes:
    * Check the box for **Run Dedicated Server**
    * Check the box for **Spatial Networking**
    > From this drop-down menu it is possible to increase the number of servers that will be launched. For now leave this at 1.
1. Still in the Unreal Editor but this time from the SpatialOS GDK toolbar, select the green **Launch**  button (not the default Launch button from the Unreal Editor toolbar). This builds your [worker configuration]({{URLRoot/content/glossary#worker-configuration) file and launches your game in a local deployment. <br/>
**Launch** opens up a terminal window and runs two SpatialOS command line interface ([CLI]({{URLRoot/content/glossary#cli)) commands: `spatial build build-config` and `spatial local launch`. It is finished when you see `SpatialOS ready` in the terminal window.
1. On the main Unreal toolbar, click **Play**. 
1. From the SpatialOS GDK toolbar click **Inspector** which will open a local [SpatialOS inspector](https://docs.improbable.io/reference/13.3/shared/operate/inspector) in your web browser. Here you can see the entities and their components present in your deployment, updates are in real-time.
  
Job done! You have ported your Unreal game to run on SpatialOS. Move around and look at the changes reflected in your inspector.

#### How to modify the default behavior
You can modify some of the GDK settings from the Unreal Editor toolbar at **Edit** > **Project Settings** >**SpatialOS Unreal GDK** > **Toolbar**.
You can change:
*  the snapshot file's filename and location
*  the launch configuration