# Gameplay Ability System on the GDK for Unreal

The [Gameplay Ability System](https://docs.unrealengine.com/en-us/Gameplay/GameplayAbilitySystem) is a flexible framework for building abilities and attributes of the type you might find in an RPG, MMO, or MOBA. However, as it makes use of lots of the advanced parts of the Unreal networking stack, the GDK for Unreal doesn't *currently* support it in it's entirety. 

This page lists the workarounds that you need to use with the Gameplay Ability System in the alpha release of the GDK. We are working to remove some of these limitations in the future.

## Gameplay Ability System workarounds
1. The GDK does not currently support replicated gameplay abilities. To work around this, you must ensure that any `UGameplayAbility` has its `ReplicationPolicy` set to `ReplicateNo`. Replicated gameplay abilities will be available once we support dynamic Actor components in the GDK.
2. You must add any `UAttributeSet` objects to the Actor that owns the `AbilitySystemComponent`, and you must add them to the Actor as `DefaultSubObjects`. For example -

    ```
    MyActor::MyActor()
    {
        ...
        AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
        AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));
        ...
    }
    ```
    
    This ensures that the replicated data on the `UAttributeSet` object is replicated correctly.    
3. If the `AbilitySystemComponent` is owned by a class that extends `UPawn`, you must override the `UPawn::OnRep_Controller()` function and call `AbilitySystemComponent::RefreshAbilityActorInfo()`. For example -

    ```
    MyActor::OnRep_Controller()
    {
        Super::OnRep_Controller();
        AbilitySystem->RefreshAbilityActorInfo();
    }
    ```
    
This is necessary due to an edge case in the GDK for Unreal where a Pawn may be checked out from the SpatialOS Runtime before the Pawn's controller is.
