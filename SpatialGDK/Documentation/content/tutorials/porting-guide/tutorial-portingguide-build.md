

# Port your project to SpatialOS

## 2. Modify and build your project

Before you open your project in the Editor you need to: 

* Add the SpatialGDK module to your project
* Build your project using Visual Studio

<!--- **Note:**  If you built the Unreal Engine fork manually, you must clone and set up the GDK plugin by following the [manual GDK installation instructions]({{urlRoot}}/content/manual-engine-build#installing-the-spatialos-gdk-for-unreal) before you follow the rest of this guide.</br> --->

## Step 1: Add the SpatialGDK module to your project

To use the GDK and SpatialOS networking, you must add the SpatialGDK [module](https://docs.unrealengine.com/en-US/Programming/UnrealBuildSystem/ModuleFiles/index.html) to your project.

1. In **File Explorer**, navigate to `\<ProjectRoot>\<GameRoot>\Source\<YourProject>\`.
1. Open the `<YourProject>.build.cs` file in a code editor and add add `"SpatialGDK"` to `PublicDependencyModuleNames`.

For example:  

```
   PublicDependencyModuleNames.AddRange(
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

## Step 2: Build your project

Set up your Unreal project to work with the GDK Unreal Engine fork, which you cloned and installed in the [Before you start]({{urlRoot}}/content/tutorials/tutorial-porting-guide#before-you-start) section. 

To do this:

1. In **File Explorer**, navigate to `<ProjectRoot>\<GameRoot>`.
1. Right-click your `<YourProject>.uproject` file and select **Switch Unreal Engine version**.
1. Select the path to the Unreal Engine fork you cloned earlier. This associates your project with the Unreal Engine fork and automatically generates a Visual Studio solution file for your project called `<YourProject.sln>`
1. In the same directory, double-click `<YourProject>`.sln to open it with Visual Studio.
1. On the Visual Studio toolbar, set your Solution configuration to **Development Editor**. <br/>
![GDK for Unreal Documentation]({{assetRoot}}assets/porting-guide/porting-solution-config.png)<br/>
 _Image: The Visual Studio toolbar, with the Development Editor Solution configuration highlighted in red._
1. In the Solution Explorer window, right-click on **`<YourProject>`** and select **Build**.

## Step 3: Modify Unreal classes for GDK compatibility

You must modify your `GameInstance` class to work with the GDK.  

Make your `GameInstance` inherit from `SpatialGameInstance`.  <br/>

* If you have not made a `GameInstance` for your game and are still using the default `GameInstance`, you must either create a Blueprint or a native `GameInstance` class now. Remember to configure your `Project Settings` to use this new `GameInstance` by default, under **Project Settings > Project Maps and Modes > Game Instance > Game Instance Class**. <br/>

* If your game's `GameInstance` is a C++ class, locate its header file and add the following `#include`:
  `"SpatialGameInstance.h"`

  For example:

  ```cpp
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

- If your `GameInstance` is a Blueprint class, you need to open and edit it in the Blueprint Editor: 

      * From the Blueprint Editor toolbar, navigate to the **Class Settings**. In **Class Options** set the **Parent Class** to `SpatialGameInstance`.

  ![spatial game instance reparent]({{assetRoot}}assets/porting-guide/spatial-game-instance-reparent.png)<br/>
    _Image: The Blueprint class settings screen_<br/>
    
## Step 4: Modify your project to support SpatialOS cloud deployments
Before you launch a cloud deployment, you need to make sure that `spatial` directory gets [cooked](https://docs.unrealengine.com/en-US/Engine/Deployment/Cooking) when you build your workers. To do this, open DefaultGame.ini (located in <ProjectRoot>\<GameRoot>\Config\DefaultGame.ini) configuration file in a text editor, and add the following two lines at the end of the file:

```
[/Script/UnrealEd.ProjectPackagingSettings]
+DirectoriesToAlwaysCook=(Path="Spatial")
```

  
 #### **> Next:** [3. Launch a local deployment]({{urlRoot}}/content/tutorials/porting-guide/tutorial-portingguide-deployment)
 
  
<br/>

<br/>------------<br/>2019-07-16 Page updated with editorial review.<br/>


