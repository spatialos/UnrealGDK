<%(TOC)%>
# Multiserver Shooter Tutorial

## Step 3: Test your changes locally

1. In Unreal Editor, in the SpatialOS GDK toolbar, select **Start**. It's ready when you see `SpatialOS ready. Access the inspector at [http://localhost:21000/inspector]()`.
1. From the Unreal Editor toolbar, click **Play** to run the game.

Notice that health now decrements when you are shot.

<br/>
### View your SpatialOS world in the Inspector

![]({{assetRoot}}assets/tutorial/inspector-two-workers.png)

_Image: A local Inspector showing two server-worker instances (two Unreal servers) managing your game_<br/>

[The Inspector]({{urlRoot}}/content/glossary#inspector) provides a real-time view of what is happening in your [SpatialOS world]({{urlRoot}}/content/glossary#game-world). It’s a powerful tool for monitoring and debugging both during development and when your game is live in production. Let’s use the Inspector to visualise the areas that each of our server-worker instances have [authority]({{urlRoot}}/content/glossary#authority) (that is, read and write access) over.

1. Access the Inspector at [http://localhost:21000/inspector](http://localhost:21000/inspector).
1. In the **View** tab, check the boxes next to both of the **UnrealWorkers**.
1. In the **Show me** option, select **Authority / interest**.<br>
    This causes the Inspector to display the areas that each server-worker instance has authority over as two colored zones.
1. Back in your two Unreal game clients, run around and shoot.
1. Using the Inspector to track the location of your two players, notice that if you position them in the same area of authority then their shots damage each other, but if they are on different servers, they can’t damage each other. Let’s fix that.

<br/>
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

Because you have changed code in a function, you now need to rebuild your project. Additionally, because you've enabled replication for a variable, you need to generate schema. To do this:

1. Open **ThirdPersonShooter.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **ThirdPersonShooter** and select **Build**.
1. Open **ThirdPersonShooter.uproject** in the Unreal Editor and click `Schema` and then `Snapshot`.

Now let’s test our new cross-server functionality in another local deployment.

<br/>
### Deploy the project locally (last time)

1. In Unreal Editor, in the SpatialOS GDK toolbar, select **Start**. It's ready when you see `SpatialOS ready. Access the inspector at [http://localhost:21000/inspector]()`.
1. From the Unreal Editor toolbar, click **Play** to run the game.
1. Using the Inspector to track the location of your two players, notice that you can now shoot between two Unreal servers and cause damage across their boundaries (provided the two players are on different teams!).

![]({{assetRoot}}assets/tutorial/shooting-across-boundaries.gif)<br/>
*Image: Players running and shooting between two Unreal Servers*

Now that you're free of the single-server paradigm, have a think about the huge, seamless multiplayer worlds you can build and host using the Unreal GDK.

Speaking of hosting, let’s upload your game.

[Step 4: Test your changes in the cloud]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-cloudtest)
