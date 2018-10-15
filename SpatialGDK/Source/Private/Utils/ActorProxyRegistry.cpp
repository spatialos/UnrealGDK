// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ActorProxyRegistry.h"

#include "EngineUtils.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogActorProxyRegistry);

void UActorProxyRegistry::AddToRegistry(const FUnrealObjectRef& ObjectRef, AActor* Actor)
{
	ObjectRefToActor.Add(ObjectRef, Actor);
	ActorToObjectRef.Add(Actor, ObjectRef);
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
