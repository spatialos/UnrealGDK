// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ActorProxyRegistry.h"

#include "EngineUtils.h"
#include "SpatialConstants.h"
#include "Schema/StandardLibrary.h"
#include "SpatialTypebindingManager.h"
#include "SpatialNetDriver.h"
#include "SpatialWorkerConnection.h"

DEFINE_LOG_CATEGORY(LogActorProxyRegistry);

void UActorProxyRegistry::AddToRegistry(const FUnrealObjectRef& ActorProxyObjectRef, AActor* OwnerActor)
{
	ObjectRefToActor.Add(ActorProxyObjectRef, OwnerActor);
	ActorToObjectRef.Add(OwnerActor, ActorProxyObjectRef);

	// Send a component update with the new query
	improbable::Interest InterestComponent;

	improbable::ComponentInterest::Query NewQuery;
	NewQuery.Constraint.EntityIdConstraint = ActorProxyObjectRef.Entity;
	NewQuery.ResultComponentId.Add(SpatialConstants::UNREAL_METADATA_COMPONENT_ID);
	NewQuery.Frequency = 10.0f;

	improbable::ComponentInterest NewCompnentInterest;
	NewCompnentInterest.Queries.Add(NewQuery);

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	USpatialTypebindingManager* TypebindingManager = NetDriver->TypebindingManager;
	FClassInfo* Info = TypebindingManager->FindClassInfoByClass(OwnerActor->GetClass());
	check(Info);

	// TODO: Determine more accurately which components we should have access to
	//InterestComponent.ComponentInterest.Add(Info->RPCComponents[RPC_Client], NewCompnentInterest);
	//InterestComponent.ComponentInterest.Add(Info->RPCComponents[RPC_Server], NewCompnentInterest);
	//InterestComponent.ComponentInterest.Add(Info->RPCComponents[RPC_CrossServer], NewCompnentInterest);
	//InterestComponent.ComponentInterest.Add(Info->RPCComponents[RPC_NetMulticast], NewCompnentInterest);

	Worker_ComponentUpdate NewInterestQueryUpdate = InterestComponent.CreateInterestUpdate();
	NetDriver->Connection->SendComponentUpdate(OwnerActor->UnrealObjectRef.Entity, &NewInterestQueryUpdate);
}

void UActorProxyRegistry::RemoveFromRegistry(const AActor* Actor)
{
	FUnrealObjectRef ObjectRef = GetObjectRefFromActor(Actor);
	RemoveFromRegistryImpl(Actor, ObjectRef);
}

void UActorProxyRegistry::RemoveFromRegistry(const FUnrealObjectRef& ObjectRef)
{
	AActor* Actor = GetActorFromObjectRef(ObjectRef);
	RemoveFromRegistryImpl(Actor, ObjectRef);
}

FUnrealObjectRef UActorProxyRegistry::GetObjectRefFromActor(const AActor* Actor) const
{
	if (ActorToObjectRef.Contains(Actor))
	{
		return *ActorToObjectRef.Find(Actor);
	}

	return SpatialConstants::NULL_OBJECT_REF;
}

AActor* UActorProxyRegistry::GetActorFromObjectRef(const FUnrealObjectRef& ObjectRef) const
{
	if (ObjectRefToActor.Contains(ObjectRef))
	{
		return *ObjectRefToActor.Find(ObjectRef);
	}

	return nullptr;
}

void UActorProxyRegistry::RemoveFromRegistryImpl(const AActor* Actor, const FUnrealObjectRef& ObjectRef)
{
	ActorToObjectRef.Remove(Actor);
	ObjectRefToActor.Remove(ObjectRef);
}
