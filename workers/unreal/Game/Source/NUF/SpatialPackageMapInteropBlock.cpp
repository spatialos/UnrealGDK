// Fill out your copyright notice in the Description page of Project Settings.

#include "SpatialPackageMapInteropBlock.h"
#include "MetadataAddComponentOp.h"
#include "SpatialNetDriver.h"
#include "EntityRegistry.h"
#include "SpatialPackageMapClient.h"

void USpatialPackageMapInteropBlock::Init(UEntityRegistry* Registry)
{
	EntityRegistry = Registry;
}

void USpatialPackageMapInteropBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
	PendingEntities.Emplace(AddEntityOp.EntityId);

	if (NextBlock)
	{
		NextBlock->AddEntity(AddEntityOp);
	}
}

void USpatialPackageMapInteropBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
	if (NextBlock)
	{
		NextBlock->RemoveEntity(RemoveEntityOp);
	}
}

void USpatialPackageMapInteropBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
	// if this is a Metadata component, register 
	if (AddComponentOp->ComponentId == improbable::Metadata::ComponentId)
	{
		PendingMetadataAddOps.Emplace(AddComponentOp->EntityId, Cast<UMetadataAddComponentOp>(AddComponentOp));
	}

	if (NextBlock)
	{
		NextBlock->AddComponent(AddComponentOp);
	}
}

void USpatialPackageMapInteropBlock::RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp)
{
	if (NextBlock)
	{
		NextBlock->RemoveComponent(ComponentId, RemoveComponentOp);
	}
}

void USpatialPackageMapInteropBlock::ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp)
{
	if (NextBlock)
	{
		NextBlock->ChangeAuthority(ComponentId, AuthChangeOp);
	}
}

void USpatialPackageMapInteropBlock::ProcessOps(const TWeakPtr<worker::View>& InView, const TWeakPtr<worker::Connection>& InConnection, UWorld* World, ::UCallbackDispatcher* CallbackDispatcher)
{
	/*
		For each AddMetadataComponent op we receive, we expect to eventually spawn an Actor (in most cases)
		Here we iterate over all pending AddMetadataComponent ops and attempt to find the Actor with the 
		associated EntityId. If we find an Actor in the EntityRegistry, we can then assign a NetGUID for 
		the Actor and its subobjects in the region 0 -> 0x7FFFFFFF (0 -> 2^31 - 1)
	*/

	for (auto& Component : PendingMetadataAddOps)
	{
		AActor* EntityActor = EntityRegistry->GetActorFromEntityId(FEntityId(Component.Key));
		if (EntityActor)
		{
			UE_LOG(LogTemp, Log, TEXT("Actor was found for EntityId: %ll"), Component.Key);
	
			USpatialNetDriver* Driver = Cast<USpatialNetDriver>(GetOuter());
			if (Driver->ClientConnections.Num() > 0)
			{
				// should provide a better way of getting hold of the SpatialOS client connection 
				USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(Driver->ClientConnections[0]->PackageMap);
				if (PMC)
				{
					PMC->ResolveEntityActor(EntityActor, FEntityId(Component.Key));		
				}
			}

			PendingMetadataAddOps.Remove(Component.Key);
		}
	}


	//for (auto& Entity : PendingEntities)
	//{
	//	if (EntityRegistry->EntityIdToActor.Find(PendingOp.))
	//	PendingOp
	//}
}

