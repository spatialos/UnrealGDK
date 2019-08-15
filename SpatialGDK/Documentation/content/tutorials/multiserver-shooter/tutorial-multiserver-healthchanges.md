<%(TOC)%>

# Multiserver zoning
## 1: Set up replication

Before you set up replication, make sure you have changed your Example Project branch from `release` (which is the default) to `multiserver-tutorial-start` - see the tutorial's [Introduction]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-intro#change-your-branch-of-the-example-project) for guidance.

### Step 1: Make the change
In the Example Project each `GDKCharacter` contains a `UHealthComponent` with a variable called `CurrentHealth`, which keeps track of that character's health. On your servers, `CurrentHealth` is reduced whenever a character is shot, but this reduction is not replicated on the clients connected to the game. This is because the `CurrentHealth` variable is not setup for replication.

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

### Step 2: Generate schema and rebuild your project
Because you've modified code related to replication, SpatialOS requires you to generate schema. Also, because you have changed code in your project, Unreal Engine requires you to rebuild your project. (Note: SpatialOS does not require you to generate a snapshot, this is because as you did this when you set up the Example Project and you only need to set up a snapshot once per project.) </br>

To generate schema:

* In the Unreal Editor, on the GDK toolbar, open the **Schema** drop-down menu and select **Schema (Full Scan)**.</br>
(See the [schema documentation]({{urlRoot}}/content/how-to-use-schema#how-to-generate-schema) for information on when and how to generate schema). 

To rebuild your project:

1. In Visual Studio, open **GDKShooter.sln**.
1. In the Solution Explorer window, right-click on **GDKShooter** and select **Build**.	

</br>
</br>
Now let's test the health replication in a local deployment.
</br>
</br>
#### **> Next:** [2: Test changes locally]({{urlRoot}}/content/tutorials/multiserver-shooter/tutorial-multiserver-localtest)
<br/>
<br/>


<br/>------<br/>
_2019-08-03 Page updated with limited editorial review: added change branch._</br>
_2019-08-02 Page updated with limited editorial review: updated project name and GDKCharacter name._</br>
_2019-04-30 Page updated with limited editorial review_
