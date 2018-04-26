#pragma once

#include "EntityPipelineBlock.h"

#include "NoOpEntityPipelineBlock.generated.h"

class UCallbackDispatcher;

namespace worker
{
struct AddEntityOp;
struct RemoveEntityOp;
struct RemoveComponentOp;
class View;
class Connection;
}

class UAddComponentOpWrapperBase;
class UEntityPipeline;

UCLASS(BlueprintType)
class SPATIALGDK_API UNoOpEntityPipelineBlock : public UEntityPipelineBlock
{
  GENERATED_BODY()

public:
  virtual void AddEntity(const worker::AddEntityOp& AddEntityOp);
  virtual void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp);

  virtual void AddComponent(UAddComponentOpWrapperBase* AddComponentOp);
  virtual void RemoveComponent(const worker::ComponentId ComponentId,
							   const worker::RemoveComponentOp& RemoveComponentOp);

  virtual void ChangeAuthority(const worker::ComponentId ComponentId,
							   const worker::AuthorityChangeOp& AuthChangeOp);

private:
  void ProcessOps(const TWeakPtr<SpatialOSView>& InView,
				  const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World,
				  UCallbackDispatcher* InCallbackDispatcher) override;
};