<%(TOC)%>

# Simulated players

## Introduction

A simulated player is a client-worker instance that is controlled by simulated player logic as opposed to real player input. You can use simulated players to scale-test your game. 

You set up simulated players by creating a character class specifically for them, and adding custom behaviour to this class. For example, you could create a player character class for simulated players and add custom movement input to it. 

You launch simulated players inside a standalone cloud deployment, and they connect to a target cloud deployment.

There is no built-in logic for simulated players. However, you can look at the [Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) implementation for reference, or use the [Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) as a starting point to implement simulated player behavior in your own project.

## Setting up simulated players

To set up simulated players, you need to:

0. [Specify a class](#1-specify-a-class-for-simulated-players) for simulated players.
0. [Set `IsSimulated` to `true`](#2-set-issimulated-to-true-for-that-class) for that class.
0. [Define the logic](#3-define-the-logic-for-your-simulated-players) for your simulated players.

### 1. Specify a class for simulated players
Inside your Game Mode constructor, specify a character class to be used for simulated players. This can be any character class, but we recommend that the simulated player character class extends your game’s player character class, so that simulated player characters inherit all the properties of your default player character class. In the following examples we will assume that you have created a character class named `SimulatedPlayerCharacter_BP` for simulated players.

You can specify the simulated player character class either in your C++ GameMode class or in the Class Defaults tab of your GameMode’s Blueprint class.

For example, in C++:

```
ATP_SpatialGDKGameMode::ATP_SpatialGDKGameMode()
{
	// Set default pawn class to our Blueprint character.
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/StarterProject/Characters/PlayerCharacter_BP"));
	static ConstructorHelpers::FClassFinder<APawn> SimulatedPawnBPClass(TEXT("/Game/StarterProject/Characters/SimulatedPlayers/SimulatedPlayerCharacter_BP"));

	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	if (SimulatedPawnBPClass.Class != NULL)
	{
		SimulatedPawnClass = SimulatedPawnBPClass.Class;
	}
}
```

### 2. Set `IsSimulated` to `true` for that class
In the Unreal Editor, within your simulated player character class's Class Defaults, set the property `IsSimulated` to `true`.

### 3. Define the logic for your simulated players
The simulated player character class is automatically spawned for simulated players instead of the default player character class. You can define custom logic for simulated players in this class. For example, you can spawn an AIController with your simulated player logic in the `BeginPlay` method, and have the controller possess the simulated player character.

As an example, you could create a new project using the [Starter Template] ({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) and look at `SimulatedPlayerCharacter_BP`:

<%(Lightbox title ="Simulated player Blueprint" image="{{assetRoot}}assets/screen-grabs/simulated-players/sim-player-blueprint.png")%>
<br>_Image: `SimulatedPlayerCharacter_BP` from the Starter Template project_

## Altering client-side logic
You might want to alter some client-side simulated player logic that is not directly related to the simulated player’s character. For example, you might want to bypass a login menu, cinematics, or a tutorial. 

To do this, you can use a Blueprint function library that exposes the `IsSimulatedPlayer` function and returns a boolean indicating whether the game client is a simulated player. This can be executed in both C++ and Blueprints.

### C++ example

```cpp
#include "SimulatedPlayer.h"


void AExampleActor::BeginPlay()
{
    Super::BeginPlay();

    if (USimulatedPlayer::IsSimulatedPlayer(this))
    {
        // Execute simulated player related logic here.
        // ...
    }
}
```

### Blueprint example

![Calling Is Simulated Player in Blueprints]({{assetRoot}}assets/screen-grabs/simulated-players/is-simulated-player.png)
<br>_Image: You can call **Is Simulated Player** from any Blueprint class, and it will return true if the current process is a simulated player client._

## Launching simulated player deployments

### Worker configuration file

To launch a simulated player deployment, you need to provide a [worker configuration file]({{urlRoot}}/content/glossary#worker-configuration-file) with the following name:
`spatialos.SimulatedPlayerCoordinator.worker.json`. This will tell SpatialOS how to build, launch, and interact with the simulated player coordinator worker, which will run the simulated players. 

The [Example Project]({{urlRoot}}/content/get-started/example-project/exampleproject-intro) and [Starter Template]({{urlRoot}}/content/get-started/starter-template/get-started-template-intro) already include this worker configuration file in the right location, but for new projects you need to add it yourself to the `spatial/workers/<worker>` directory. 

We recommend that you copy [our example configuration file](https://github.com/spatialos/UnrealGDK/blob/release/SpatialGDK/Build/Programs/Improbable.Unreal.Scripts/WorkerCoordinator/SpatialConfig/spatialos.SimulatedPlayerCoordinator.worker.json), and adapt it to your project where necessary. This file contains the arguments that are passed to the simulated player game clients. For information on customising this configuration file, please visit [this page](https://docs.improbable.io/reference/14.2/shared/project-layout/introduction).

Once you have copied the file, you'll need to run `spatial build build-config` from the `spatial` directory of your project. This will apply this new configuration to your project's executable.

### Local deployments

There are two ways to launch simulated players that connect to local deployments, and you can choose which one to use. 

- **Launch simulated players using the Multiplayer Options window**
</br>
  You can launch simulated players as [Play In Editor](https://docs.unrealengine.com/en-US/GettingStarted/HowTo/PIE/index.html#playineditor) clients by configuring the “Number of Simulated Players” option (**Edit > Editor Preferences > Level Editor > Play > Multiplayer Options**):
  </br></br>
  ![Number of simulated players]({{assetRoot}}assets/screen-grabs/simulated-players/multiplayer-options.png)
  <br>_Image: In the "Multiplayer Options" section, enter the number of simulated players_
  </br></br>

- **Launch simulated players automatically using batch scripts**
</br>
  You can launch simulated players that connect to local deployments using a batch file. The Example Project contains `LaunchSimPlayerClient.bat` which launches a single Simulated Player client, and `Launch10SimPlayerClients.bat` which launches 10. If you use simulated players to test your project locally we recommend using these scripts as inspiration for your own, as they can save you time.

### Cloud deployments

You can scale test your game right from the start by having simulated players connect to your cloud deployment. For more information, see [Launch cloud deployment]({{urlRoot}}/content/cloud-deployment-workflow#launch-cloud-deployment).

## Logs
Logs for simulated players are stored as [raw logs](https://docs.improbable.io/reference/latest/shared/worker-configuration/raw-worker-logs) in the simulated player deployment.

<br/>
<br/>------------<br/>
_2019-11-28 Page updated without editorial review: added batch scripts to launch simulated players._<br/>
_2019-11-14 Page updated without editorial review: added missing `spatial build build-config` step._<br/>
_2019-07-31 Page added with limited editorial review._
