# Gameplay Ability System on the GDK for Unreal

The [Gameplay Ability System](https://docs.unrealengine.com/en-us/Gameplay/GameplayAbilitySystem) is a highly flexible framework for building abilities and attributes of the type you might find in an RPG, MMO, or MOBA. However, as it makes use of lots of the advanced parts of the Unreal networking stack, the GDK for Unreal doesn't *currently* support it in it's entirety. On this page we'll list the workarounds that are required if you want to use the Ability System with the Alpha release of the GDK. We are working to remove some of these limitations in the future.

## Current Gameplay Ability Workarounds
1. We currently do not support Replicated Gameplay Abilities. Ensure that any `UGameplayAbility` has its `ReplicationPolicy` set to `ReplicateNo`. Replicated Gameplay Abilities will be available once we support dynamic actor components in the GDK.
1. Ensure all `UAttributeSet` objects are added as `DefaultSubObjects` on the `Actor` that owns the `AbilitySystemComponent`. For instance -
    ```
    MyActor::MyActor()
    {
        ...
        AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystem"));
        AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));
        ...
    }
    ```
    This will ensure the replicated data on the `UAttributeSet` object is correctly replicated.    
1. If the `AbilitySystemComponent` is owned by a class that extends `UPawn`, override the `UPawn::OnRep_Controller()` function and call `AbilitySystemComponent::RefreshAbilityActorInfo()`. For instance -
    ```
    MyActor::OnRep_Controller()
    {
        Super::OnRep_Controller();
        AbilitySystem->RefreshAbilityActorInfo();
    }
    ```
This is required due to an edge case in the GDK for Unreal where a Pawn may be checked out from the runtime before the Pawn's controller is.
