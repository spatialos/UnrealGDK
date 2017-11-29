// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EntityPipelineBlock.h"
#include "EntityId.h"
#include "SpatialPackageMapInteropBlock.generated.h"

class UAddComponentOpWrapperBase;
class UMetadataAddComponentOp;
class UCallbackDispatcher;
class UEntityRegistry;
class UEntityPipeline;
class USpatialOsComponent;

/**
	We want to keep MetadataAddComponentOps and then use their data to populate the PackageMap for Actors that have been spawned
	
 
 */
UCLASS()
class NUF_API USpatialPackageMapInteropBlock : public UEntityPipelineBlock
{
	GENERATED_BODY()
	
public:
	void Init(UEntityRegistry* Registry);

	void AddEntity(const worker::AddEntityOp& AddEntityOp) override;
	void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp) override;

	void AddComponent(UAddComponentOpWrapperBase* AddComponentOp) override;
	void RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp) override;

	void ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp) override;

private:
	void ProcessOps(const TWeakPtr<worker::View>& InView,
		const TWeakPtr<worker::Connection>& InConnection, UWorld* World,
		::UCallbackDispatcher* CallbackDispatcher) override;

	UPROPERTY()
	UEntityRegistry* EntityRegistry;

	UPROPERTY()
	TMap<uint64, UMetadataAddComponentOp*> PendingMetadataAddOps;

	UPROPERTY()
	TSet<FEntityId> PendingEntities;
};
