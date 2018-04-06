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
class SPATIALOS_API UNoOpEntityPipelineBlock : public UEntityPipelineBlock
{
  GENERATED_BODY()

public:
  virtual void AddEntity(const worker::AddEntityOp& AddEntityOp) override;
  virtual void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp) override;

  virtual void AddComponent(UAddComponentOpWrapperBase* AddComponentOp) override;
  virtual void RemoveComponent(const worker::ComponentId ComponentId,
                               const worker::RemoveComponentOp& RemoveComponentOp) override;

  virtual void ChangeAuthority(const worker::ComponentId ComponentId,
                               const worker::AuthorityChangeOp& AuthChangeOp) override;

  virtual void EnterCriticalSection() override;
  virtual void LeaveCriticalSection() override;

private:
  void ProcessOps(const TWeakPtr<SpatialOSView>& InView,
                  const TWeakPtr<SpatialOSConnection>& InConnection, UWorld* World,
                  UCallbackDispatcher* InCallbackDispatcher) override;
};
