#pragma once

#include "EntityPipelineBlock.h"
#include "EmptyPipelineBlock.generated.h"

UCLASS()
class NUF_API UEmptyPipelineBlock : public UEntityPipelineBlock
{
	GENERATED_BODY()

public:
	void AddEntity(const worker::AddEntityOp& AddEntityOp) override;
	void RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp) override;
	void AddComponent(UAddComponentOpWrapperBase* AddComponentOp) override;
	void RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp) override;
	void ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp) override;

private:
	void ProcessOps(const TWeakPtr<worker::View>& InView,
		const TWeakPtr<worker::Connection>& InConnection, UWorld* World,
		UCallbackDispatcher* CallbackDispatcher) override;
};
