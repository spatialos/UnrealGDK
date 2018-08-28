//// Fill out your copyright notice in the Description page of Project Settings.
//
//#pragma once
//
//#include "CoreMinimal.h"
//#include "UObject/NoExportTypes.h"
//#include "Legacy/EntityRegistry.h"
//#include "SpatialNetDriver.h"
//
//#include "SpatialActorSpawner.generated.h"
//
//using ComponentStorageBase = worker::detail::ComponentStorageBase;
//
//struct FAddComponent
//{
//	worker::ComponentId ComponentId;
//	TSharedPtr<ComponentStorageBase> Component;
//};
//
//UCLASS()
//class SPATIALGDK_API USpatialActorSpawner : public UObject
//{
//	GENERATED_BODY()
//	
//public:
//	void Init(USpatialNetDriver* NetDriver, UEntityRegistry* EntityRegistry);
//
//	template <typename T>
//	void Accept();
//
//	void RegisterCallbacks();
//	
//	void AddEntity(const worker::AddEntityOp& op);
//	void RemoveEntity(const worker::RemoveEntityOp& op);
//	void HitCriticalSection(const worker::CriticalSectionOp& op);
//	
//	void CreateActor(const worker::EntityId& EntityId);
//	AActor* SpawnActor(improbable::PositionData* PositionComponent, UClass* ActorClass, bool bDeferred);
//	void DeleteActor(const worker::EntityId& EntityId);
//
//	void CleanupDeletedActor(const worker::EntityId EntityId);
//
//	UClass* GetNativeEntityClass(improbable::MetadataData* MetadataComponent);
//
//	bool IsInCriticalSection() { return inCriticalSection; }
//
//private:
//	TArray<worker::AddEntityOp> PendingAddEntityOps;
//	TArray<worker::RemoveEntityOp> PendingRemoveEntityOps;
//	TMap<worker::EntityId, TArray<FAddComponent>> PendingAddComponentOps;
//
//	UEntityRegistry* EntityRegistry;
//
//	USpatialNetDriver* NetDriver;
//	TSharedPtr<worker::Connection> Connection;
//	TSharedPtr<worker::View> View;
//
//	UWorld* World;
//
//	bool inCriticalSection;
//
//	template <typename Metaclass>
//	typename Metaclass::Data* GetComponentDataFromView(const worker::EntityId& EntityId)
//	{
//		auto EntityIterator = View->Entities.find(EntityId);
//		if (EntityIterator == View->Entities.end())
//		{
//			return nullptr;
//		}
//		return EntityIterator->second.Get<Metaclass>().data();
//	}
//};
