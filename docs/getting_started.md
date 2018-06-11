# Getting started
To get going using the Unreal GDK, you need to install and set up:
* the SpatialOS fork of the Unreal Engine
* the Unreal GDK module and Sample Game <br/>
(TODO: How to set up with your own game [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-233))

Note that there are two set up (build) options for the Unreal GDK module and Sample game; **Standard development** or **Unreal GDK modification development**. You need to choose one of them in the instructions below.

## Table of contents
* [Prerequisites](#Prerequisites)
* [Setting up the SpatialOS fork of the Unreal Engine](#Setting_up_the_SpatialOS_fork_of_the_Unreal_Engine)
* [Setting up the Unreal GDK module and Sample Game](#Setting_up_the_Unreal_GDK_module_and_Sample_Game)
    * [Download](#Download)
    * [Build](#Build) 
        * [Build for standard development](Build_for_standard_development)
        * [Build for Unreal GDK modification development ](Build_for_Unreal_GDK_modification_development)
* [Run the Sample Game](Run_the_Sample_Game)
---

## Prerequisites

**Hardware** 
* Refer to [UE4 recommendations](https://docs.unrealengine.com/en-US/GettingStarted/RecommendedSpecifications) (Unreal documentation)
* Recommended storage: 15GB+ available space


**Network settings**
* Refer to the [SpatialOS network settings](https://docs.improbable.io/reference/latest/shared/get-started/requirements#network-settings) (SpatialOS documentation)

**Software**

To build the SpatialOS Unreal GDK module you need the following installed on your computer:
* Windows 10 (We haven’t tested this on Windows 7.)
* [Git-for-windows](gitforwindows.org)
* [Golang v1.9+](https://golang.org/)
* [SpatialOS version 13](https://docs.improbable.io/reference/13.0/shared/get-started/setup/win)<br/> (Information on downloading and installing SpatialOS is on the [SpatialOS website](https://docs.improbable.io/reference/13.0/shared/get-started/tour))
* [Improbable toolshare](https://brevi.link/toolshare) <br/>
(TODO: fix tool availability for external users - [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-290))
* The Unreal engine built from `UnrealEngine419_SpatialGDK` branch of our [forked Unreal Engine repository](https://github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK). <br/>
(See [Setting up the SpatialOS fork of the Unreal Engine](#setting-up-the-spatialos-fork-of-the-unreal-engine) below for details. See also [Engine changes](#engine-changes), below for notes.)<br/>
 (TODO: Fix repo link for external users - [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))

**Notes:** 
* The Unreal GDK runs on Windows only.
* Currently, you can only build the GDK if you are an Improbable employee. 
This is due to the dependency on Improbable toolshare. We will remove this dependency, either;
	*  When we completely remove the GDK’s reliance on the earlier-developed [Unreal SDK](https://github.com/spatialos/UnrealSDK), or
	* if we fork [imp_lint](https://github.com/improbable/platform/tree/master/go/src/improbable.io/cmd/imp_lint) and [imp_nugget](https://github.com/improbable/platform/tree/master/go/src/improbable.io/cmd/imp-nuget) from the [SpatialOS platform repo](https://github.com/improbable/platform). <br/>
    (TODO: Remove for external users -  [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-290))

---

## Setting up the SpatialOS fork of the Unreal Engine

1. Download and install the SpatialOS Unreal engine from our Unreal engine fork [github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK](https://github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK).<br/> (TODO: Fix link for external users [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))
	* Make sure that you checkout the [UnrealEngine419_SpatialGDK](https://github.com/improbable/UnrealEngine/tree/UnrealEngine419_SpatialGDK) branch to get the correct version.
	* See the Unreal Engine fork [readme](https://github.com/improbable/UnrealEngine/blob/UnrealEngine419_SpatialGDK/README.md) file for guidance on setting up the SpatialOS Unreal engine. Follow steps 1 to 4 in the `UnrealEngine419_SpatialGDK` readme. 
    (TODO check install  for external users [JIRA ticket](https://improbableio.atlassian.net/browse/UNR-263))
    (TODO: Fix link for external users [JIRA TICKET](https://improbableio.atlassian.net/browse/UNR-304))
2. Create an `UNREAL_HOME` environment variable. To do this:
	1.  In Windows open the **Control Panel**, select **System Properties** and then **Advanced System Settings**. 
	2. In the **Advanced** dialog box select **Environment Variables...** tab to see the Environment Variables dialog box. 
    3. Select **New…** to see the **Edit System Variables** dialog box. Complete the fields as below:<br/>
|   **Variable Name** | ` UNREAL_HOME`<br/>
|   **Variable value** |  `<the path from your computer’s root directory to the SpatialOS fork of Unreal engine you have just downloaded>`

---

## Setting up the Unreal GDK module and Sample Game

Follow the steps below to:
*  Download the Unreal GDK module and Sample Game.
*  Build the Unreal GDK module dependencies which the Sample Game needs so it can  work with the GDK and add the Unreal GDK to the sample game.

Note that there are two build options:
* **Standard development**: follow these instructions if you want to develop games with the Unreal GDK and do not want to modify the GDK.
* **Unreal GDK modification development**: follow these instructions if you want to modify the Unreal GDK while developing games with it.

### Download
Download the Unreal GDK module and Sample Game; with [Git](gitforwindows.org) installed, clone the [Unreal GDK repository](https://github.com/improbable/unreal-gdk)  and the [Sample Game repository](https://github.com/improbable/unreal-gdk-sample-game/) from GitHub.<br/>
(To use Git from the command line, see GitHub documentation’s [Setting up Git](https://help.github.com/articles/set-up-git/) and [Fork a Repo](https://help.github.com/articles/fork-a-repo/) articles.)

### Build 
Build the Unreal GDK module dependencies which the Sample Game needs to work with the GDK and add the Unreal GDK to the sample game. Choose one of the two build options.

#### Build for standard development
Follow these instructions if you want to develop games with the Unreal GDK and do not want to modify the GDK.

1. Build the GDK module dependencies by opening a terminal window and running the following script from the root of the Unreal GDK repository you just cloned run: <br/> 
`ci/build.sh`.
2. Initialise the project, from the same directory run:<br/> `Game/Scripts/Codegen.bat`.
3. Add the Unreal GDK to the Sample Game, from the root of the GDK repository directory run: <br/>
`setup.sh <path to your Sample Game root folder>`. 
<br/>
(This copies the `Binaries`, `Script`, `SpatialGDK` and `Plugins/SpatialGDK` folders from the Unreal GDK repository to the correct locations in your Sample Game repository.)
4. Set the Sample Game to work with the SpatialOS fork of UE4 you cloned and installed earlier. To do this:
	1. Locate the Unreal project file for the Sample Game,`SampleGame.uproject`. This is under the Sample Game's root directory in `Game`.
    2. Right-click on `SampleGame.uproject` and from the drop-down menu select **Switch Unreal Engine version...**
    3. Select the path of the SpatialOS Unreal engine version you downloaded earlier (UnrealEngine419).
6. Build the SpatialOS Unreal engine, from a terminal window in any directory run:<br/>
`Game/Scripts/Build.bat SampleGameEditor Win64 Development Game/SampleGame.uproject`.


**Notes:** 
* If you make changes to your user defined classes, you will need to re-run the [InteropCodeGenerator](how_the_unreal_gdk_works.md#interopcodegenerator).
* The interop code and schema generated for marshalling updates and RPCs gets committed directly to the source tree in `Game/Source/SampleGame/Generated` and `schema/improbable/unreal/generated` respectively. This means you only need to re-run the commandlet if you have changed the code generator.
* There are two non-load-balanced launch scripts to assist your development:
    * `one_worker_test.json` tests that managed workers launch correctly and ensures that entities never cross worker boundaries.
    * `two_worker_test.json` provides a static non-overlapping worker boundary between two workers to assist your entity migration testing. As worker boundaries don't overlap, workers have no knowledge of an entity which is under the authority of a different worker.

#### Build for Unreal GDK modification development 
Follow these instructions if you want to modify the Unreal GDK while developing games with it.*

1.   Build the GDK module by opening a terminal window and running the following script  from the root of the Unreal GDK repository you just cloned: Run `ci/build.sh`
2. Initialise the project: from the same directory, run `Game/Scripts/Codegen.bat`.
3. Create symlinks between the Sample Game and the Unreal GDK repository by running the following script from within the Sample Game repository you just cloned:   Run `create_spatialGDK_symlink.bat <path to Unreal GDK root>`
4. Set the Sample Game to work with the SpatialOS fork of UE4 you cloned and installed earlier:
	1. Locate `SampleGame.uproject`, the Unreal project file for the Sample Game. This is under the Sample Game's root directory in  in `Game`.
	2. Right-click on `SampleGame.uproject` and select **Switch Unreal Engine version...**
5. Select the path of the SpatialOS Unreal engine version (UnrealEngine419) you downloaded earlier.
6. Build the SpatialOS Unreal engine by running the following command from a terminal window in any directory: Run `Game/Scripts/Build.bat SampleGameEditor Win64 Development Game/SampleGame.uproject`.

**NOTE:** If you make changes to your user defined classes, you will need to run the [InteropCodeGenerator](how_the_unreal_gdk_works.md#interopcodegenerator).

---

### Run the Sample Game

1. Initialise the project from the root directory of your project, run `Game/Scripts/Codegen.bat`.
1. Start SpatialOS local deployment by running `spatial local launch <launch configuration>`.
1. Locate the `SampleGame.uproject` project file. It’s under your project’s root directory in the `Game/` directory.
1. Double-click on `SampleGame.uproject` to open the Unreal Editor.
1. In the editor, select the **Play** drop-down menu to see **Modes** and **Multiplayer Options**.
1. From the drop-down menu, select **New Editor Window (PIE)**. Enter **Number of Players** as `2` and check the box for **Run Dedicated Server**.
1. Run the game by clicking **Play** on the editor toolbar.
