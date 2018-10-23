<%(Callout type="warn" message="This [pre-alpha](https://docs.improbable.io/reference/latest/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)")%>

# How to port a native Unreal project to the Unreal GDK

## Prerequisites

<%(TOC)%>

Follow the [setup guide]({{urlRoot}}/setup-and-installing) to the end of **Setting up the Unreal GDK module and Starter Project** > **Cloning**.
> To speed up build times we recommend installing IncrediBuild, FastBuild, or another build distributor.

## Glossary
`<ProjectRoot>` - The root folder of your Unreal project.  
`<GameRoot>` - The folder containing your Unreal game files and source code. Usually called ‘Game’ e.g. `<ProjectRoot>/Game`  
`<YourProject>` - Name of your games `.uproject` e.g. `StarterProject.uproject`  

## Setting up the project structure
1. Navigate to the root directory of the SpatialOS Unreal GDK repository you cloned (this should be `<ProjectRoot>\<GameRoot>\Plugins\UnrealGDK\`) and run `Setup.bat` in a terminal window or by double-clicking the file. This requires authorization with your SpatialOS account via a web-browser.
2. We now require some files and folders for the SpatialOS Unreal GDK to run correctly with your project, we are going to obtain these by copying them from the `StarterProject` repo that you cloned earlier. Open the StarterProject repo and copy all of the files and folders except for the `\Game\` folder into your `<ProjectRoot>` folder. Your folder structure should now resemble:
```
\<ProjectRoot>\ci\
\<ProjectRoot>\<GameRoot>\
\<ProjectRoot>\spatial\
\<ProjectRoot>\ProjectPaths.bat
\<ProjectRoot>\LaunchClient.bat 
etc...
```
1. Open **` \<ProjectRoot>\ProjectPaths.bat`** for editing and replace `Game` (located at `set PROJECT_PATH=Game`) with your `<GameRoot>` folder name, and `StarterProject` (located at `set GAME_NAME=StarterProject`) with the name of your games uproject (which we'll refer to as `<YourProject>`).

## Adding the SpatialGDK module to your project
1. In your project's `*.build.cs` file, add "SpatialGDK" to the `PublicDependencyModuleNames`.
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

## Modifying Unreal classes for GDK compatibility
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

1. In your project's `PlayerController` header file(s), add the public method declaration
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

## Adding Unreal GDK configurations
1. In `<GameRoot>\Config`, open `DefaultEngine.ini` and add:

    ``` ini
    [/Script/Engine.Engine]
    !NetDriverDefinitions=ClearArray
    +NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SpatialGDK.SpatialNetDriver",DriverClassNameFallback="/Script/SpatialGDK.SpatialNetDriver")

    [/Script/SpatialGDK.SpatialNetDriver]
    NetConnectionClassName="/Script/SpatialGDK.SpatialNetConnection"
    ```

    > These lines ensure that the SpatialOS Unreal GDK can override Unreal's network replication.
    >
    > If you ever need to run with default Unreal networking for workflow or validation reasons, un-tick `Spatial Networking` in the `Play` drop down menu in the Unreal editor. This flag is maintained in your Unreal config files under `/Script/EngineSettings.GeneralProjectSettings` and is also valid for non-editor builds.
    >
    > You can easily switch between networking modes using this checkbox. As the SpatialOS Unreal GDK is in pre-alpha, switching back to Unreal's networking may be a viable way to work around some issues you may encounter, and may speed up your iteration. However, in Unreal networking mode you lose access to key multi-server features of the GDK, which may lead to erratic behavior.
    >
    > In the future, we expect to achieve parity with Unreal networking in terms of iteration speed and stability, thus removing the need for a networking mode switch.

2. To provide context to SpatialOS on how to replicate singleton classes (such as GameState and GameMode) we have added a `UCLASS` macro which provides said context. The next steps detail how to do this. 
   
3. If your games '`GameState`' is a cpp file, locate it and mark it as a public singleton by modifying the UCLASS macro as shown:
    ```
    UCLASS(SpatialType=Singleton)
    ```
    If your games '`GameState`' is a blueprint then you will need to open the full blueprint editor for that GameState and navigate to the `Class Settings` in the blueprint editor toolbar. In the `Class Options` tick the `Spatial Type` checkbox and add the tag `Singleton` to the `Spatial Description` textbox.

4. If your games '`GameMode`' is a cpp file, locate it and mark it as a private singleton by modifying the UCLASS macro as shown:
    ```
    UCLASS(SpatialType=(Singleton,ServerOnly))
    ```
    If your games '`GameMode`' is a blueprint then you will need to open the full blueprint editor for that GameMode and navigate to the `Class Settings` in the blueprint editor toolbar. In the `Class Options` tick the `Spatial Type` checkbox and add the tag `Singleton,ServerOnly` to the `Spatial Description` textbox.

5. Marking these singleton classes as `Spatial Type` will cause spatial schema to be generated for said classes when pressing the `Schema` button in the `SpatialOSGDKToolbar`. We now need to mark the generated schema components as streaming queries for your UnrealWorker worker configuration, this ensures your UnrealWorkers always have these singletons checked out. The generated schema is located at `<ProjectRoot>\spatial\schema\improbable\unreal\generated`.

    Open the UnrealWorker worker config located in `<ProjectRoot>\spatial\workers\unreal\spatialos.UnrealWorker.worker.json` and add your new Singleton generated component(s) to the streaming queries.
    
    For example:

            "streaming_query": [
            {
                "global_component_streaming_query": {
                "component_name": "improbable.unreal.GlobalStateManager"
                }
            },
            {
                "global_component_streaming_query": {
                "component_name": "improbable.unreal.generated.yourprojectgamestate.YourProjectGameStateMultiClientRepData"
                }
            },
            {
                "global_component_streaming_query": {
                "component_name": "improbable.unreal.generated.yourprojectgamemode.YourProjectGameModeMultiClientRepData"
                }
            }
            ],
    
## Generating code for SpatialOS C++ workers and building your project
Set your Unreal project to work with the SpatialOS GDK for Unreal fork of the Unreal Engine, which you cloned and installed earlier. To do this:
1. In File Explorer, navigate to `<ProjectRoot>\<GameRoot>`.
2. Right-click your uproject file and select **Switch Unreal Engine version**.
3. Select the path to the Unreal Engine fork you cloned earlier.
4. In a terminal window, from `<ProjectRoot>\<GameRoot>\Plugins\UnrealGDK\SpatialGDK\Build\Scripts`, run `BuildWorker.bat <YourProject>Editor Win64 Development <YourProject>.uproject`   
For Example "`BuildWorker.bat StarterProjectEditor Win64 Development StarterProject.uproject`"  
This builds and packages your workers ready for a Spatial deployment.
5. In File Explorer, right-click your uproject file and select **Generate Visual Studio project files**.
6. Open the solution in Visual Studio, and compile and run the project in the Development Editor configuration.

## Running the Schema and Snapshot Generator
1. In the Unreal Editor, on the SpatialOS Unreal GDK toolbar, click  the **Schema** button to run the `Schema Generator`.
2. On the same toolbar, click the **Snapshot** button.

## Running your game
1. In the Unreal Editor, on the toolbar, open the **Play** drop-down menu and check the box next to **Run Dedicated Server** as well as the checkbox for **Spatial Networking**
2. On the SpatialOS Unreal GDK toolbar, click the green **Launch** (not the default Launch button in the Unreal Editor), `spatial local launch` will be ran in a separate console window, wait for the output `SpatialOS ready`.
3. On the main Unreal toolbar, click **Play**.

## Modifying default behavior
If you wish to modify the default behavior for the SpatialOS GDK for Unreal; such as the default snapshot save path and filename, the spatial launch configuration used when pressing **Launch** and more... you can modify these settings from the Unreal Editor by navigating to `Edit->Project Settings->SpatialOS Unreal GDK->Toolbar`. 