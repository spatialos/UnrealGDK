// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "NoOpEntityPipelineBlock.h"

#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "Engine.h"
#include "SpatialGDKViewTypes.h"
#include "SpatialGDKWorkerTypes.h"

void UNoOpEntityPipelineBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
  UE_LOG(LogTemp, Log, TEXT("AddEntityOp received"));
}

void UNoOpEntityPipelineBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
  UE_LOG(LogTemp, Log, TEXT("RemoveEntityOp received"));
}

void UNoOpEntityPipelineBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
  UE_LOG(LogTemp, Log, TEXT("AddComponentOp received"));
}

void UNoOpEntityPipelineBlock::RemoveComponent(const worker::ComponentId ComponentId,
											   const worker::RemoveComponentOp& RemoveComponentOp)
{
  UE_LOG(LogTemp, Log, TEXT("RemoveComponentOp received"));
}

void UNoOpEntityPipelineBlock::ChangeAuthority(const worker::ComponentId ComponentId,
											   const worker::AuthorityChangeOp& ChangeAuthority)
{
  UE_LOG(LogTemp, Log, TEXT("ChangeAuthorityOp received"));
}

void UNoOpEntityPipelineBlock::ProcessOps(const TWeakPtr<SpatialOSView>& InView,
										  const TWeakPtr<SpatialOSConnection>& InConnection,
										  UWorld* World, UCallbackDispatcher* InCallbackDispatcher)
{
}
