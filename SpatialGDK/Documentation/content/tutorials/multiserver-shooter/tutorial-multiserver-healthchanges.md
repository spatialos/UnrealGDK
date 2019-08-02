<%(TOC)%>
# Multiserver Shooter Tutorial
## 2: Replicate health changes</br>
### Step 1: Make the change


In this project each `GDKCharacter` contains a `UHealthComponent` with a variable called `CurrentHealth`, which keeps track of that character's health. On your servers, `CurrentHealth` is reduced whenever a character is shot, but this reduction is not replicated on the clients connected to the game. This is because the `CurrentHealth` variable is not setup for replication.

To resolve this you need to mark the `CurrentHealth` property for replication, just as you would in the native [Unreal Actor replication](https://docs.unrealengine.com/en-us/Resources/ContentExamples/Networking/1_1) workflow. To do this:

1. In your IDE, open `UnrealGDKExampleProject\Game\Source\GDKShooter\Public\Characters\Components\HealthComponent.h`.
1. Navigate to the declaration of the `CurrentHealth` variable, and add the UProperty specifiers `ReplicatedUsing = OnRep_CurrentHealth`. The UProperty should now look like this:

```
    UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
    float CurrentHealth; 
```

    You have now marked this property for replication using the `OnRep_CurrentHealth` function that you’ll implement in the next section.
    
    Next you need to update the [GetLifetimeReplicatedProps](https://wiki.unrealengine.com/Replication#Actor_Property_Replication) implementation of the `TPSCharacter` to specify the [replication conditions](https://docs.unrealengine.com/en-US/Gameplay/Networking/Actors/Properties/Conditions) for the `CurrentHealth` variable:

1. In your IDE, open `UnrealGDKExampleProject\Game\Source\GDKShooter\Private\Characters\Components\HealthComponent.cpp`.
1. Navigate to the `GetLifetimeReplicatedProps` function (which is implementd around line 182), and insert the following snippet:

```
    // Only replicate health to the owning client.
    DOREPLIFETIME(UHealthComponent, CurrentHealth);
```

    Finally, you need to implement the `OnRep_CurrentHealth` function so that the player health UI gets updated when the `CurrentHealth` variable is replicated:

1. In your IDE, open `UnrealGDKExampleProject\Game\Source\GDKShooter\Public\Characters\Components\HealthComponent.h`.
1. In the public scope of the class, insert the following snippet:

    ```	
        UFUNCTION()
        void OnRep_CurrentHealth();
    ```

1. In your IDE, open `UnrealGDKExampleProject\Game\Source\GDKShooter\Private\Characters\Components\HealthComponent.cpp` and insert the following snippet:

```
    void UHealthComponent::OnRep_CurrentHealth()
    {
	    HealthUpdated.Broadcast(CurrentHealth, MaxHealth);
	    if (CurrentHealth <= 0.f)
	    {
		    Death.Broadcast();
	    }
    }
```

</br>
Notice that the workflow you just used mirrors that of native Unreal.

### Step 2: Rebuild your project
Because you have changed code in a function, you now need to rebuild your project. Additionally, because you've modified code related to replication, you need to generate schema. To do this:

1. Open **GDKShooter.sln** with Visual Studio.
1. In the Solution Explorer window, right-click on **GDKShooter** and select **Build**.
1. Open **GDKShooter.uproject** in the Unreal Editor and click `Schema` and then `Snapshot`.

Now let’s test our health replication in another local deployment.

</br>
</br>
**> Next:** [3: Test changes locally]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-localtest)
<br/>
<br/>


<br/>------<br/>
_2019-08-02 Page updated with limited editorial review_