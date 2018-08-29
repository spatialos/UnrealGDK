> This [pre-alpha](https://docs.improbable.io/reference/13.1/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)

# Porting a native Unreal project to the Unreal GDK

> This workflow is very subject to change. We're aware that it's not optimized, and one of our priorities is to improve iteration times.

## Prerequisites

<%(TOC)%>

Follow the [setup guide]({{urlRoot}}/setup-and-installing) to the end of **Setting up the Unreal GDK module and Starter Project** > **Cloning**.
> To speed up build times, install IncrediBuild, FastBuild, or another build distributor.

## Setting up the project structure
1. In a terminal window, navigate to the root directory of the SpatialOS Unreal GDK repository you cloned and run `BuildGDK.bat`. This requires authorization with your SpatialOS account.
1. In File Explorer, make a copy of the SpatialOS Unreal GDK Starter Project repository you cloned. This copy will become the root directory of the Unreal project you're porting over, so paste it where you want this project to be.
1. Rename the copied directory so it's relevant to your Unreal game. We'll refer to this directory as `<ProjectRoot>`.
1. Within `<ProjectRoot>`, replace the `Game` directory with the equivalent directory for your Unreal game (we'll refer to this as `<GameRoot>`).
1. Open **`ProjectPaths.bat`** for editing and replace `Game` with `<GameRoot>`, and `StarterProject` with the name of your uproject (which we'll refer to as `<YourProject>`).
1. Create symlinks between your Unreal project and the Unreal GDK:
    1. Open another instance of **File Explorer** and locate the Unreal GDK directory (but don't open it).
    1. Drag the Unreal GDK directory onto the batch script **`CreateGDKSymlinks.bat`** (located within `<ProjectRoot>`).
This brings up a terminal window, and the output should be something like `Successfully created symlinks to "C:\Users\name\Documents\UnrealGDK"`.
For more information on helper scripts, see [Helper scripts](https://github.com/spatialos/UnrealGDKStarterProject#helper-scripts) in the Starter Project readme.

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

1. In your project's uproject file, add the following to the `Modules` section:
    ``` csharp
    {
        "Name": "SpatialGDK",
        "Type": "Runtime",
        "LoadingPhase": "Default"
    }
    ```
    For example:
    ``` csharp
    "Modules": [
            {
                "Name": "YourProject",
                "Type": "Runtime",
                "LoadingPhase": "Default"
            },
            {   "Name": "SpatialGDK",
                "Type": "Runtime",
                "LoadingPhase": "Default"
            }
        ],
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

1. In your project's `PlayerController` header file(s), add the method declaration
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
    > If you ever need to run with Unreal networking for workflow or validation reasons, simply add `-NetDriverOverrides=/Script/OnlineSubsystemUtils.IpNetDriver` to your command line arguments when you're launching the game (to do this in the Visual Studio Solution Explorer, right-click the project and select **Properties**, then **Debugging**, and add to the list of Command Arguments).
    >
    > You can easily switch between networking modes using this argument. As the SpatialOS Unreal GDK is in pre-alpha, switching back to Unreal's networking may be a viable way to work around some issues you may encounter, and may speed up your iteration. However, in Unreal networking mode you lose access to key multi-server features of the GDK, which may lead to erratic behavior.
    >
    > In the future, we expect to achieve parity with Unreal networking in terms of iteration speed and stability, thus removing the need for a networking mode switch.
1. Open the game's uproject file in the Editor and navigate to *Edit > Project Settings... > SpatialOS Unreal GDK > Toolbar*.

1. In the Interop Codegen section:
    1. Modify the output path for the Interop code generation to: `<ProjectRoot>/spatial/schema/improbable/generated/`.
    1. Modify the output path for the generated schemas to: `<GameRoot>/Source/<GameName>/Generated/`.

1. Tag all replicated classes with the UCLASS tag `SpatialType`:
    ``` 
    UCLASS(SpatialType) 
    ```
1. Locate your game's `GameState`, and make it a public singleton:
    ```
    UCLASS(SpatialType=Singleton)
    ```
1. Locate your game's `GameMode`, and make it a private singleton:
    ```
    UCLASS(SpatialType=(Singleton,ServerOnly))
    ```
    
1. Open your UnrealWorker worker config located in `<ProjectRoot>\spatial\workers\unreal\spatialos.UnrealWorker.worker.json`. Add your new Singleton generated component(s) to the streaming queries.
    
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
1. In your terminal, navigate to `<ProjectRoot>\<GameRoot>\Scripts` and run `Codegen.bat`.
This initializes the project. It should succeed quickly and silently.
1. Set your Unreal project to work with the SpatialOS Unreal GDK fork of the Unreal Engine, which you cloned and installed earlier. To do this:
    1. In File Explorer, navigate to `<ProjectRoot>\<GameRoot>`.
    1. Right-click your uproject file and select **Switch Unreal Engine version**.
    1. Select the path to the Unreal Engine fork you cloned earlier.
    1. In your terminal, from `<ProjectRoot>\<GameRoot>\Scripts`, run `BuildWorker.bat <YourProject>Editor Win64 Development <YourProject>.uproject`
    1. In File Explorer, right-click your uproject file and select **Generate Visual Studio project files**.
    1. Open the solution in Visual Studio, and compile and run the project in the Development Editor configuration.

## Running the Interop Code Generator and setting up the UE4 Editor
1. In the Unreal Editor, on the SpatialOS Unreal GDK toolbar, click **Codegen** to run the [Interop Code Generator]({{urlRoot}}/content/interop).
1. On the same toolbar, click **Snapshot**.
1. Go to **Edit** > **Project Settings** > **Maps & Modes** and change the Game Default Map to match what's set for the Editor Startup Map. (This is a temporary requirement.)
1. Close the Unreal Editor.
1. Compile and run again with the same build configuration (new source files that require compilation will have been generated when you ran the Interop Code Generator).

## Running your game
1. In the Unreal Editor, on the toolbar, open the **Play** drop-down menu and check the box next to **Run Dedicated Server**.
1. On the SpatialOS Unreal GDK toolbar, click **Launch** and wait for the output `SpatialOS ready`.
1. On the main Unreal toolbar, click **Play**.
