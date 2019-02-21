<%(TOC)%>
# Next steps: Set up the GDK with the Starter Project

Before starting this, you need to have followed:

* [Getting started: 1 - Dependencies]({{urlRoot}}/content/get-started/dependencies)
* [Getting started: 2 - Get and build the GDK’s Unreal Engine Fork]({{urlRoot}}/content/get-started/build-unreal-fork)

This step has three parts; clone the repositories, build the dependencies and deploy the game. You can deploy locally on your development machine and in the cloud.

### Clone the repositories

Now, you are going to clone the Starter Project repository and then clone the GDK repository into your copy of the Starter Project. Note that the GDK needs to go into the Starter Project's `Plugins` directory.

1. Clone the [Starter Project](https://github.com/spatialos/UnrealGDKStarterProject/) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDKStarterProject.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDKStarterProject.git`
1. Navigate into `<StarterProjectRepoRoot>\Game\` and create a `Plugins` folder.
1.  In a Git Bash terminal window, navigate to `<StarterProjectRepoRoot>\Game\Plugins` and clone the [GDK for Unreal](https://github.com/spatialos/UnrealGDK) repository by running either:
    * (HTTPS) `git clone https://github.com/spatialos/UnrealGDK.git`
    * (SSH) `git clone git@github.com:spatialos/UnrealGDK.git`

**Note** You need to ensure that the root folder of the GDK for Unreal repository is called `UnrealGDK` so its path is: `<StarterProjectRepoRoot>\Game\Plugins\UnrealGDK\...`

[//]: # (TODO: This whole section below deserves some screenshots to show users what to expect. This was done well on the Unity onobarding docs. Example: https://docs.improbable.io/unity/alpha/content/get-started/get-playing)	

### Build dependencies 

In this step, you're going to build the GDK for Unreal module dependencies, which the Starter Project needs to work with the GDK. You are then going to add the GDK for Unreal to the Starter Project.

1. Open **File Explorer**, navigate to the root directory of the GDK for Unreal repository, and double-click **`Setup.bat`**. You may be prompted to sign into your SpatialOS account if you have not already.
1. Set the Starter Project to work with the Unreal Engine fork you cloned and installed earlier. To do this:
    1. In File Explorer, navigate to the root directory of the GDK for Unreal Starter Project repository, and then to the **Game** directory within it.
    1. Right-click **StarterProject.uproject** and select **Switch Unreal Engine version**.
    1. Select the path to the Unreal Engine fork you cloned earlier.
1. In the same directory, open **StarterProject.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **StarterProject** and select **Build**.
1. Open **StarterProject.uproject** in the Unreal Editor 
    * Click [`Schema` (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary) to generate schema 
    
    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/schema-button.png)

    * Click [`Snapshot` (SpatialOS documentation)](https://docs.improbable.io/reference/latest/shared/glossary) to generate a snapshot.

    ![Toolbar]({{assetRoot}}assets/screen-grabs/toolbar/snapshot-button.png)

### Deploy the Starter Project 

#### Deploy locally

1. In the Unreal Editor, on the SpatialOS GDK toolbar, click **Start**. Wait until you see the output `SpatialOS ready. Access the inspector at http://localhost:21000/inspector`.
1. On the Unreal Editor toolbar, open the **Play** drop-down menu.
1. Under **Multiplayer Options**, enter the number of players as **2** and check the box next to **Run Dedicated Server**. Then, under Modes, select **New Editor Window (PIE)**.
1. On the toolbar, click **Play** to run the game.
1. When you're done, click **Stop** to stop the [client-workers and server-workers]({{urlRoot}}/content/glossary#workers), and click the **Stop** button in the SpatialOS section of the toolbar to stop your local SpatialOS deployment.

If you would like to run multiple server-workers in editor, see the [Toolbar documentation]({{urlRoot}}/content/toolbars#launching-multiple-pie-server-workers) regarding launching multiple PIE server-workers

#### Deploy in the cloud

To run a cloud deployment, you need to prepare your server-worker and client-worker [assemblies](https://docs.improbable.io/reference/latest/shared/glossary), and upload them to the cloud.

> Building the assemblies can take a while - we recommend installing <a href="https://www.incredibuild.com/" data-track-link="Incredibuild|product=Docs|platform=Win|label=Win" target="_blank">IncrediBuild</a> to speed up build times.

1. Change the name of the project
    1. In File Explorer, navigate to the root directory of the Starter Project repository, then to **`spatial`**, and open the `spatialos.json` file in an editor of your choice.
    1. Change the `name` field to the name of your project. You can find this in the [Console](https://console.improbable.io). It’ll be something like `beta_someword_anotherword_000`.
1. In a terminal window, navigate to the root directory of the Starter Project repository.
1. Build a server-worker assembly: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat StarterProjectServer Linux Development StarterProject.uproject`
1. Build a client-worker assembly: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat StarterProject Win64 Development StarterProject.uproject`
1. Navigate to `StarterProject/spatial`.
1. Upload the assemblies to the cloud, specifying an assembly name (this includes both client-worker and server-worker assemblies): `spatial cloud upload <assembly_name>`
1. Launch a deployment, specifying a deployment name: `spatial cloud launch <assembly_name> one_worker_test.json <deployment_name> --snapshot=snapshots\default.snapshot`
1. On your deployment's page in the [SpatialOS Web Console](https://console.improbable.io) click Play to launch the game.

### Congratulations!

You've successfully set up and launched the Starter Project and the GDK! If you haven't already, we recommend following our Multiserver Shooter tutorial where you can try out the GDK’s development experience by adding a new feature to a project, and test it across two servers.

If you have an existing Unreal multiplayer project, you can follow our detailed [porting guide]({{urlRoot}}/content/get-started/porting-unreal-project-to-gdk.md) to get it onto the GDK.

You are now ready to add multiserver functionality on top of the Starter Project. Check out the documentation on [cross-server RPCs]({{urlRoot}}/content/cross-server-rpcs), [handover]({{urlRoot}}/content/handover-between-server-workers) and [Singleton Actors]({{urlRoot}}/content/singleton-actors).

We will be releasing more tutorials and examples as the GDK matures. Stay tuned!

