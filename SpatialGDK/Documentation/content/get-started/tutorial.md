<%(TOC)%>
# Get started: 3 - Follow the Multiserver Shooter tutorial

### What will be covered?

In this tutorial you’ll implement cross server remote procedure calls (RPCs) in a simple third person shooter. The end result will be a multiplayer, cloud-hosted Unreal game running across multiple [server-workers]({{urlRoot}}/content/glossary#inspector) that players can seamlessly move between and shoot across. It will look something like this:

![]({{assetRoot}}assets/tutorial/cross-server.gif)

The exercise demonstrates that the workflows and iteration speed you’re used to as an Unreal developer are almost entirely unaltered by the GDK: it’s just like regular Unreal!

Let’s get started.

### Clone the Unreal GDK Third Person Shooter repository

Clone the Unreal GDK Third Person Shooter repository and checkout the tutorial branch using one of the following commands:

**SSH:** `git clone git@github.com:spatialos/UnrealGDKThirdPersonShooter.git -b tutorial`

**HTTPS:** `git clone https://github.com/spatialos/UnrealGDKThirdPersonShooter.git -b tutorial`

This repository contains a version of Unreal’s Third Person template that has been ported to the SpatialOS GDK. It includes a character model with a gun and hit detection logic.

> **Note:**  A completed version of this tutorial is available in the `tutorial-complete` branch.

### Move the GDK into the `Plugins` directory


1. Navigate to `UnrealGDKThirdPersonShooter\Game` and create a `Plugins` directory.
1. In a terminal window,  change directory to the  `Plugins` directory and clone the [Unreal GDK](https://github.com/spatialos/UnrealGDK) repository using one of the following commands:

**SSH:**  `git clone git@github.com:spatialos/UnrealGDK.git`

**HTTPS:** `git clone https://github.com/spatialos/UnrealGDK.git`

The GDK's [default branch (GitHub documentation)](https://help.github.com/en/articles/setting-the-default-branch) is `release`. This means that, at any point during the development of your game, you can get the latest release of the GDK by running `git pull` inside the `UnrealGDK` directory. When you pull the latest changes, you must also run `git pull` inside the `UnrealEngine` directory, so that your GDK and your Unreal Engine fork remain in sync.

> **Note:**  You need to ensure that the root folder of the Unreal GDK repository is called `UnrealGDK` so its path is: `UnrealGDKThirdPersonShooter\Game\Plugins\UnrealGDK\`.

### Build Dependencies 

In this step, you're going to build the Unreal GDK's dependencies.

1. Open **File Explorer**, navigate to the root directory of the Unreal GDK repository, and double-click **`Setup.bat`**. You may be prompted to sign into your SpatialOS account if you have not already.
1. In `UnrealGDKThirdPersonShooter\Game`, right-click on **ThirdPersonShooter.uproject** and select **Switch Unreal Engine version**.
1. Provide the path to the Unreal Engine fork you cloned earlier.
1. In the same directory, open **ThirdPersonShooter.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **ThirdPersonShooter** and select **Build**.
1. Open **ThirdPersonShooter.uproject** in the Unreal Editor.
1. In the [GDK toolbar]({{urlRoot}}/content/toolbars), click [`Schema`]({{urlRoot}}/content/schema) to generate schema and then [`Snapshot`]({{urlRoot}}/content/glossary#snapshot) to generate a snapshot.

### Deploy the project locally

In this section you’ll run a [local deployment](https://docs.improbable.io/reference/latest/shared/glossary#local-deployment) of the project. As the name suggests, local deployments run on your development machine (you will [cloud deploy](https://docs.improbable.io/reference/latest/shared/glossary#cloud-deployment) later in this tutorial).

1. In a text editor, open `UnrealGDKThirdPersonShooter\spatial\default_launch.json` and in the `layer_configurations` section set `rows` to `2`. This instructs SpatialOS that you will be launching two server-workers.
1. In the Unreal Editor, in the Unreal toolbar, open the **Play** drop-down menu.
1. Under **Multiplayer Options**, enter the number of players as **2**
1. Enter the number of servers as **2**
1. Check the box next to **Run Dedicated Server**
1. In Unreal Editor, in the SpatialOS GDK toolbar, select **Start**, the green play icon. This will open a terminal window and run the [`spatial local launch`](https://docs.improbable.io/reference/latest/shared/spatial-cli/spatial-local-launch#spatial-local-launch) command, which starts the [SpatialOS Runtime](https://docs.improbable.io/reference/latest/shared/glossary#the-runtime). It's ready when you see `SpatialOS ready. Access the inspector at http://localhost:21000/inspector`.
1. From the Unreal Editor toolbar, click **Play** to run the game. This starts two headless server-workers and two [client-workers](https://docs.improbable.io/reference/latest/shared/glossary#client-worker).

Notice that when players shoot each other, their health does not go down. It's not much fun with no skin in the game is it? Let’s fix the health system.

### Replicate health changes

In this ThirdPersonShooter project each `TPSCharacter` contains a variable called `CurrentHealth`, which keeps track of that character's health. On your servers, `CurrentHealth` is reduced whenever a character is shot, but this reduction is not replicated on the clients connected to the game. This is because the `CurrentHealth` variable is not setup for replication.

To resolve this you need to mark the `CurrentHealth` property for replication, just as you would in the native [Unreal Actor replication](https://docs.unrealengine.com/en-us/Resources/ContentExamples/Networking/1_1) workflow. To do this:

1. In your IDE, open `UnrealGDKThirdPersonShooter\Game\Source\ThirdPersonShooter\Characters\TPSCharacter.h`.
1. Navigate to the declaration of the `CurrentHealth` variable, and add the UProperty specifiers `ReplicatedUsing = OnRep_CurrentHealth`. The UProperty should now look like this:

    ```
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    int32 CurrentHealth; 
    ```

    You have now marked this property for replication using the `OnRep_CurrentHealth` function that you’ll implement in the next section.
    
    Next you need to update the [GetLifetimeReplicatedProps](https://wiki.unrealengine.com/Replication#Actor_Property_Replication) implementation of the `TPSCharacter` to specify the [replication conditions](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties/Conditions) for the `CurrentHealth` variable:
    
1. In your IDE, open `UnrealGDKThirdPersonShooter\Game\Source\ThirdPersonShooter\Characters\TPSCharacter.cpp`.
1. Navigate to the `GetLifetimeReplicatedProps` function (which is implementd around line 182), and insert the following snippet:

    ```
    // Only replicate health to the owning client.
    DOREPLIFETIME_CONDITION(ATPSCharacter, CurrentHealth, COND_OwnerOnly);
    ```

    > **Note:** You only want to replicate the `CurrentHealth` variable to the client that owns this Actor, thus you specify the `COND_OwnerOnly` flag.

    Finally, you need to implement the `OnRep_CurrentHealth` function so that the player health UI gets updated when the `CurrentHealth` variable is replicated:

1. In your IDE, open `UnrealGDKThirdPersonShooter\Game\Source\ThirdPersonShooter\Characters\TPSCharacter.h`.
1. In the public scope of the class, insert the following snippet:
    
    ```	
    UFUNCTION()
    void OnRep_CurrentHealth();
    ```

1. In your IDE, open `UnrealGDKThirdPersonShooter\Game\Source\ThirdPersonShooter\Characters\TPSCharacter.cpp` and insert the following snippet:

    ```
    void ATPSCharacter::OnRep_CurrentHealth()
    {
    	if (GetNetMode() != NM_DedicatedServer)
    	{
		ATPSPlayerController* PC = Cast<ATPSPlayerController>(GetController());
    		if (PC)
    		{
    			PC->UpdateHealthUI(CurrentHealth, MaxHealth);
    		}
    		else
    		{
    			UE_LOG(LogTPS, Warning, TEXT("Couldn't find a player controller for character: %s"), *this->GetName());
    		}
    	}
    }
    ```

Notice that the workflow you just used mirrors that of native Unreal.

Because you’ve changed code in a function you now need to build your project, generate schema and a new snapshot. To do this:

1. Open **ThirdPersonShooter.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **ThirdPersonShooter** and select **Build**.
1. Open **ThirdPersonShooter.uproject** in the Unreal Editor and click `Schema` and then `Snapshot`.

Now let’s test our health replication in another local deployment.

### Deploy the project locally (again)

1. In Unreal Editor, in the SpatialOS GDK toolbar, select **Start**. It's ready when you see `SpatialOS ready. Access the inspector at [http://localhost:21000/inspector]()`.
1. From the Unreal Editor toolbar, click **Play** to run the game.

Notice that health now decrements when you are shot.

### View your SpatialOS world in the Inspector

![]({{assetRoot}}assets/tutorial/inspector.png)

[The Inspector]({{urlRoot}}/content/glossary#inspector) provides a real-time view of what is happening in your [SpatialOS world]({{urlRoot}}/content/glossary#game-world). It’s a powerful tool for monitoring and debugging both during development and when your game is live in production. Let’s use the Inspector to visualise the areas that each of our server-workers have [authority]({{urlRoot}}/content/glossary#authority) (that is, read and write access) over.

1. Access the inspector at [http://localhost:21000/inspector](http://localhost:21000/inspector).
1. In the **View** tab, click the checkboxes next to both of the **UnrealWorkers**. This will cause the Inspector to display the areas that the server-workers have authority over as two coloured zones.
1. Back in your two Unreal game clients, run around and shoot.
1. Using the Inspector to track the location of your two players, notice that if you position them in the same area of authority then their shots damage each other, but if they are on different servers, they can’t damage each other. Let’s fix that.

### Enable cross server RPCs

To damage a player on a different server, the actor shooting the bullet must send a cross-server RPC to the actor getting hit by the bullet. You will implement this by overriding the [TakeDamage (Unreal documentation)](https://api.unrealengine.com/INT/API/Runtime/Engine/GameFramework/APawn/TakeDamage/index.html) function in the TPSCharcter class.

1. In your IDE, open `UnrealGDKThirdPersonShooter\Game\Source\ThirdPersonShooter\Characters\TPSCharacter.h`.
1. On line 74, add this snippet:

    ```
    UFUNCTION(CrossServer, Reliable)
    void TakeGunDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
    ```

    This snippet creates a new `UFUNCTION` marked with the function tags [CrossServer]({{urlRoot}}/content/cross-server-rpcs) and [Reliable (Unreal documentation)](https://wiki.unrealengine.com/Replication#Reliable_vs_Unreliable_Function_Call_Replication). The CrossServer tag forces this function to be executed as a cross-server RPC.

1. In your IDE, open `UnrealGDKThirdPersonShooter\Game\Source\ThirdPersonShooter\Characters\TPSCharacter.cpp`.
1. Replace the `TakeDamage` function (lines 514-548) with this snippet:

```
float ATPSCharacter::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TakeGunDamageCrossServer(Damage, DamageEvent, EventInstigator, DamageCauser);

	return Damage;
}

void ATPSCharacter::TakeGunDamageCrossServer_Implementation(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return;
	}

	const ATPSCharacter* Killer = nullptr;

	// Ignore friendly fire
	const AInstantWeapon* DamageSourceWeapon = Cast<AInstantWeapon>(DamageCauser);
	if (DamageSourceWeapon != nullptr)
	{
		const ATPSCharacter* DamageDealer = Cast<ATPSCharacter>(DamageSourceWeapon->GetWeilder());
		if (DamageDealer != nullptr)
		{
			if (Team != ETPSTeam::Team_None    // "Team_None" is not actually a team, and "teamless" should be able to damage one another
				&& DamageDealer->GetTeam() == Team)
			{
				return;
			}
			Killer = DamageDealer;
		}
	}

	int32 DamageDealt = FMath::Min(static_cast<int32>(Damage), CurrentHealth);
	CurrentHealth -= DamageDealt;

	if (CurrentHealth <= 0)
	{
		Die(Killer);
	}
}
```

This snippet implements the functionality that was previously contained within `TakeDamage` as a cross-server RPC called `TakeGunDamageCrossServer`.

Because you’ve changed code in a function you now need build **ThirdPersonShooter.sln**, generate schema and a new snapshot. To do this:

1. Open **ThirdPersonShooter.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **ThirdPersonShooter** and select **Build**.
1. Open **ThirdPersonShooter.uproject** in the Unreal Editor and click `Schema` and then `Snapshot`.

Now let’s test our new cross-server functionality in another local deployment.

### Deploy the project locally (last time)

1. In Unreal Editor, in the SpatialOS GDK toolbar, select **Start**. It's ready when you see `SpatialOS ready. Access the inspector at [http://localhost:21000/inspector]()`.
1. From the Unreal Editor toolbar, click **Play** to run the game.
1. Using the Inspector to track the location of your two players, notice that you can now shoot across servers and cause damage.

Now that you're free of the single-server paradigm, have a think about the huge, seamless multiplayer worlds you can build and host using the Unreal GDK.

Speaking of hosting, let’s upload your game.

### Build your assemblies

An assembly is what’s created when you run `BuildWorker.bat`. They’re .zip files that contains all the files that your game uses when running in the cloud.

1. In a terminal window, change directory to the root directory of the Third Person Shooter repository.
1. Build a server-worker assembly by running: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat ThirdPersonShooterServer Linux Development ThirdPersonShooter.uproject`
1. Build a client-worker assembly by running: `Game\Plugins\UnrealGDK\SpatialGDK\Build\Scripts\BuildWorker.bat ThirdPersonShooter Win64 Development ThirdPersonShooter.uproject`

### Upload your game

1. In File Explorer, navigate to `UnrealGDKThirdPersonShooter\spatial` and open `spatialos.json` in a text editor.
1. Change the `name` field to the name of your project. You can find this in the [Console](https://console.improbable.io). It’ll be something like `beta_someword_anotherword_000`.
![]({{assetRoot}}assets/tutorial/project-name.png)
1. In a terminal window, change directory to `UnrealGDKThirdPersonShooter\spatial\` and run `spatial cloud upload <assembly_name>`, where `<assembly_name>` is a name of your choice (for example `myassembly`). A valid upload command looks like this:

```
spatial cloud upload myassembly
```

> **Note:** Based on your network speed it may take a little while (1-10 minutes) to upload your assembly.

### Launch a cloud deployment

The next step is to launch a cloud deployment using the assembly that you just uploaded. This can only be done through the SpatialOS command-line interface (also known as the [`spatial` CLI]({{urlRoot}}/content/glossary#spatial-command-line-tool-cli).

When launching a cloud deployment you must provide three parameters:

* **the assembly name**, which identifies the worker assemblies to use.
* **a launch configuration**, which declares the world and load balancing configuration.
* **a name for your deployment**, which is used to label the deployment in the [Console](https://console.improbable.io).

1. In a  terminal window, in the same directory you used to upload your game, run: `spatial cloud launch --snapshot=snapshots/default.snapshot <assembly_name> two_worker_test.json <deployment_name>` 
<br/>where `assembly_name` is the name you gave the assembly in the previous step and `deployment_name` is a name of your choice. A valid launch command would look like this:

```
spatial cloud launch --snapshot=snapshots/default.snapshot myassembly two_worker_test.json mydeployment
```

> **Note:** This command defaults to deploying to clusters located in the US. If you’re in Europe, add the `--cluster_region=eu` flag for lower latency.

### Play your game

![]({{assetRoot}}assets/tutorial/console.png)

When your deployment has launched, SpatialOS automatically opens the [Console](https://console.improbable.io) in your browser.

1. In the Console, Select the **Launch** button on the left of the page, and then click the **Launch** button that appears in the centre of the page. The SpatialOS Launcher, which was installed along with SpatialOS, downloads the game client for this deployment and runs it on your local machine.
![]({{assetRoot}}assets/tutorial/launch.png)
1. Once the client has launched, enter the game and fire a few celebratory shots - you are now playing in your first SpatialOS cloud deployment!

### Invite your friends

1. To invite other players to this game, head back to the Deployment Overview page in your [Console](https://console.improbable.io), and select the **Share button**.
1. Share the generated link with your friends!

When you’re done shooting your friends, you can click the **Stop** button in the [Console](https://console.improbable.io) to halt your deployment.

### Next steps
We hope you've enjoyed this tutorial. If you want to build a new game using the SpatialOS GDK, you should build it on top of the [SpatialOS GDK Starter template]({{urlRoot}}/content/get-started/gdk-template). If you want to port your existing game to SpatialOS, follow the [porting guide]({{urlRoot}}/content/get-started/porting-unreal-project-to-gdk).
