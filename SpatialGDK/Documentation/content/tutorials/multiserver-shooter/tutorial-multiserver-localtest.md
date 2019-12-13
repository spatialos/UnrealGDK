

# \[Experimental\] Multiserver zoning
## 2: Test changes locally
### Step 1: Start SpatialOS and start the game
1. Start SpatialOS: in Unreal Editor, in the SpatialOS GDK toolbar, select **Start**. It's ready when you see `SpatialOS Local deployment started!`.
1. Now, start the game: from the Unreal Editor toolbar, click **Play** to run the game.

As you play the game, notice that health now decrements when you are shot.

### Step 2: View your SpatialOS world in the Inspector

![]({{assetRoot}}assets/tutorial/lb-inspector-two-workers.png)<br/>
_Image: A local Inspector showing two server-worker instances (two Unreal servers) managing your game_<br/>

[The Inspector]({{urlRoot}}/content/glossary#inspector) provides a real-time view of what is happening in your [SpatialOS world]({{urlRoot}}/content/glossary#game-world). It’s a powerful tool for monitoring and debugging both during development and when your game is live in production. Let’s use the Inspector to visualise the areas that each of our server-worker instances have [authority]({{urlRoot}}/content/glossary#authority) (that is, read and write access) over.

1. Access the Inspector at [http://localhost:21000/inspector](http://localhost:21000/inspector).
1. In the **View** tab, check the boxes next to both of the **UnrealWorkers**.
1. In the **Show me** option, select **Authority / interest**.<br>
    This causes the Inspector to display the areas that each server-worker instance has authority over as two colored zones.
1. Back in your two Unreal game clients, run around and shoot.
1. Using the Inspector to track the location of your two players, notice that if you position them in the same area of authority then their shots damage each other, but if they are on different servers, they can’t damage each other. Let’s fix that.

<br/>
### Step 3: Enable cross-server RPCs

To damage a player on a different server, the actor shooting the bullet must send a cross-server RPC to the actor getting hit by the bullet. You will implement this by overriding the [TakeDamage (Unreal documentation)](https://api.unrealengine.com/INT/API/Runtime/Engine/GameFramework/APawn/TakeDamage/index.html) function in the `GDKCharcter` class.

1. In your IDE, open `UnrealGDKExampleProject\Game\Source\GDKShooter\Public\Characters\GDKCharacter.h`.
1. Under line 91, add this snippet:

    [block:code]
{
  "codes": [
  {
      "code": "    UFUNCTION(CrossServer, Reliable) \n void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);\n",
      "language": "text"
    }
  ]
}
[/block]

    This snippet creates a new `UFUNCTION` marked with the function tags [CrossServer]({{urlRoot}}/content/cross-server-rpcs) and [Reliable (Unreal documentation)](https://wiki.unrealengine.com/Replication#Reliable_vs_Unreliable_Function_Call_Replication). The CrossServer tag forces this function to be executed as a cross-server RPC.

1. In your IDE, open `UnrealGDKExampleProject\Game\Source\GDKShooter\Private\Characters\GDKCharacter.cpp`.
1. Replace the `TakeDamage` function (lines 158-163) with this snippet:

[block:code]
{
  "codes": [
  {
      "code": "float AGDKCharacter::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)\n{\nTakeDamageCrossServer(Damage, DamageEvent, EventInstigator, DamageCauser);\nreturn Damage;\n}\n\nvoid AGDKCharacter::TakeDamageCrossServer_Implementation(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)\n{\nfloat ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);\nHealthComponent->TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);\n}",
      "language": "text"
    }
  ]
}
[/block]

This snippet implements the functionality that was previously contained within `TakeDamage` as a cross-server RPC called `TakeDamageCrossServer`.

Because you have changed code in a function, you now need to rebuild your project. Additionally, because you've enabled replication for a variable, you need to generate schema. To do this:

1. Open **GDKShooter.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **GDKShooter** and select **Build**.
1. Open **GDKShooter.uproject** in the Unreal Editor and click `Schema` and then `Snapshot`.

Now let’s test our new cross-server functionality in another local deployment.

> **TIP:** Check out the [local deployment workflow page]({{urlRoot}}/content/local-deployment-workflow) for a reference diagram of this workflow.

<br/>
### Step 4: Deploy the project locally

1. In Unreal Editor, in the SpatialOS GDK toolbar, select **Start**. It's ready when you see `SpatialOS Local deployment started!`.
1. From the Unreal Editor toolbar, select **Play** to run the game.
1. Using the Inspector to track the location of your two players, notice that you can now shoot between two Unreal servers and cause damage across their boundaries (provided the two players are on different teams!).

![]({{assetRoot}}assets/tutorial/cross-server-shooting.gif)<br/>
_Image: Players running and shooting between two Unreal Servers_

</br>
</br>
Now you can upload your game to the cloud.
</br>
</br>
#### **> Next:** [3: Test changes in the cloud]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-cloudtest)
<br/>
<br/>


<br/>------<br/>
_2019-08-02 Page updated with limited editorial review: re-number steps._</br>
_2019-04-30 Page updated with limited editorial review._
