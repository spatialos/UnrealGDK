#include "EmptyPipelineBlock.h"

void UEmptyPipelineBlock::AddEntity(const worker::AddEntityOp& AddEntityOp)
{
}

void UEmptyPipelineBlock::RemoveEntity(const worker::RemoveEntityOp& RemoveEntityOp)
{
}

void UEmptyPipelineBlock::AddComponent(UAddComponentOpWrapperBase* AddComponentOp)
{
}

void UEmptyPipelineBlock::RemoveComponent(const worker::ComponentId ComponentId, const worker::RemoveComponentOp& RemoveComponentOp)
{
}

void UEmptyPipelineBlock::ChangeAuthority(const worker::ComponentId ComponentId, const worker::AuthorityChangeOp& AuthChangeOp)
{
}

void UEmptyPipelineBlock::ProcessOps(const TWeakPtr<worker::View>& InView,
	const TWeakPtr<worker::Connection>& InConnection, UWorld* World,
	UCallbackDispatcher* CallbackDispatcher)
{
}
