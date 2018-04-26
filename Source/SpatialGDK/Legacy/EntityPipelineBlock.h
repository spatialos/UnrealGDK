#pragma once

#include "CallbackDispatcher.h"
#include "ScopedViewCallbacks.h"
#include "UObject/NoExportTypes.h"

#include "EntityPipelineBlock.generated.h"

namespace worker
{
struct AddEntityOp;
struct RemoveEntityOp;
struct RemoveComponentOp;
struct AuthorityChangeOp;
}

class UAddComponentOpWrapperBase;
class UEntityPipeline;

UCLASS(Abstract)
class SPATIALGDK_API UEntityPipelineBlock : public UObject
{
	GENERATED_BODY()
	friend class UEntityPipeline;

public:
	virtual void AddEntity(const worker::AddEntityOp &AddEntityOp)
		PURE_VIRTUAL(UEntityPipelineBlock::AddEntity(), );
	virtual void RemoveEntity(const worker::RemoveEntityOp &RemoveEntityOp)
		PURE_VIRTUAL(UEntityPipelineBlock::RemoveEntity(), );

	virtual void AddComponent(UAddComponentOpWrapperBase *AddComponentOp)
		PURE_VIRTUAL(UEntityPipelineBlock::AddComponent(), );
	virtual void RemoveComponent(const worker::ComponentId ComponentId,
								 const worker::RemoveComponentOp &RemoveComponentOp)
		PURE_VIRTUAL(UEntityPipelineBlock::RemoveComponent(), );

	virtual void ChangeAuthority(const worker::ComponentId ComponentId,
								 const worker::AuthorityChangeOp &AuthChangeOp)
		PURE_VIRTUAL(UEntityPipelineBlock::ChangeAuthority(), );

	virtual void EnterCriticalSection() PURE_VIRTUAL(UEntityPipelineBlock::EnterCriticalSection(), );

	virtual void LeaveCriticalSection() PURE_VIRTUAL(UEntityPipelineBlock::LeaveCriticalSection(), );

protected:
	UPROPERTY()
	UEntityPipelineBlock *NextBlock;

	virtual void ProcessOps(const TWeakPtr<SpatialOSView> &InView,
							const TWeakPtr<SpatialOSConnection> &InConnection,
							UWorld *World,
							::UCallbackDispatcher *CallbackDispatcher)
		PURE_VIRTUAL(UEntityPipelineBlock::ProcessOps(), );
};