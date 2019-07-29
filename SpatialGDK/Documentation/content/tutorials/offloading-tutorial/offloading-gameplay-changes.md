<%(TOC)%>

# Offloading example project tutorial

## 2. Make gameplay changes

### CrossServerPawn

A common scenario when building offloaded gameplay features is when interacting actors are offloaded to different workers. Traditionally, this would have been done by either having the actor performing an action on another actor invoking a function directly on the actor. This worked fine when there was only one server maintaining the entire state of the world that has authority over all actors in the simulation. In the case of offloading, this no longer the case if the two interacting actors have their authority assigned to different worker types.

To solve this, any functions invoked by the interacting actor will need to be handled via a [Cross-server RPCs]({{urlRoot}}/content/technical-overview/gdk-concepts#cross-server-rpcs).

In the case of the example project, the turrets are set up to run on a separate worker(AIWorker). If a player shoots the turret, a call to TakeDamage is invoked which is part of the native Actor API in unreal. The TakeDamage function is not an RPC function so it would not be executed on the turrets unless they internally get routed to the AIWorker using a cross-server RPC.

In the example project, this is implmented via a class called ACrossServerPawn which implements the actor interface and overrides the implementation for TakeDamage to invoke a new cross server rpc called TakeDamageCrossServer as shown in the snippet below:

```
float ACrossServerPawn::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    TakeDamageCrossServer(Damage, DamageEvent, nullptr, DamageCauser);
     return Damage;
}

void ACrossServerPawn::TakeDamageCrossServer_Implementation(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	IncomingDamage.Broadcast(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
}
```

The turret blueprint (BP_Turret_Base) inherits from the ACrossServerPawn class.

### GDKCharacter

The CrashBot and the Player does not use the ACrossServerPawn. Instead, they implement the GDKCharacter base class which uses the same take damage override pattern to enable the cross server interaction.

### Turrets

Due to how offloading works in the GDK, level placed actors are spawned on an UnrealWorker and then replicated to the worker matching their actor group, at which point the correct worker can start simulating the entity with the correct authority configuration. This can lead to problems due to certain initialization steps occurring on the initial worker not being replicated correctly.

In the case of the turret, its controller is not a replicated actor and therefore, its creation needs to be deferred until authority is gained over its SpatialOS entity on the AIWorker. This was done by hooking up the controller creation to the OnAuthorityGained delegate on the turret (See image below). In addition to this, the Auto Possess Player and Auto Possess AI settings the blueprint details panel had to be set to be disabled to prevent them from being created at launch.

![img]({{assetRoot}}assets/offloading-project/turrets.png)

### CrashBot

Another thing that needs to be taken into consideration when implementing gameplay features with offloading is that actor groups are defined on a class level. This means that if you define a weapon class that your player and you later want to allow your offloaded AI characters to use that weapon as well, then you may run into issues as the weapon can only have its authority assigned to one worker. 

The solution to this is to create a new subclass of the weapon that can be assigned to the actor group that the offloaded AI belongs to.

In the example project,  the Player characters and the CrashBots have the same base class, we had to create a child blueprint called BP_CrashBot to ensure that the offloading configuration setup correctly.

In addition to the CrashBot blueprint, the example project had to create an additional version of the CrashBot named  BP_CrashBot_PlacedInWorld. This was to work around the issue of creating controllers first when the authority was gained.

Mention weapon to allow it to be used for the bots.

### Ensure that code get executed on the correct server worker

With the introduction of offloading, it is no longer enough to check whether the netmode is a dedicated server to ensure that logic is run on the authoritative worker. An actor could now be authoritatively updated from any given server type and therefore it is important to be mindful of checking whether the current server worker has authority over the entity. Below are a few example snippets from the UEquipmentComponent where such checks were added to ensure correct flow:

```
void UEquippedComponent::SpawnStarterTemplates(FGDKMetaData MetaData)
{
	if (!bHeldItemsInitialised && GetOwner()->HasAuthority())
	{
	...

void UEquippedComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetNetMode() == NM_DedicatedServer && GetOwner()->HasAuthority())
	{
	...
```

### Conclusion

Offloading enables you to scale your CPU heavy tasks by offloading them to other worker types without having to worry about the challenges involved with migrating entities across worker boundaries.

Offloading does introduce a set of constraints to be mindful of, however, when set up correctly it will offer you significant scaling opportunities.

<br/>------------<br/>
_2019-07-26 Page added with limited editorial review_
<br/>
<br/>
[//]: # (TODO: https://improbableio.atlassian.net/browse/DOC-1142)
------------