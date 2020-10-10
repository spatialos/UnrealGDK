#include "SpatialView/CommandRequest.h"

namespace SpatialGDK
{
CommandRequest::CommandRequest(FComponentId ComponentId, FCommandIndex CommandIndex)
	: ComponentId(ComponentId)
	, CommandIndex(CommandIndex)
	, Data(Schema_CreateCommandRequest())
{
}

CommandRequest::CommandRequest(OwningCommandRequestPtr Data, FComponentId ComponentId, FCommandIndex CommandIndex)
	: ComponentId(ComponentId)
	, CommandIndex(CommandIndex)
	, Data(MoveTemp(Data))
{
}

CommandRequest CommandRequest::CreateCopy(const Schema_CommandRequest* Data, FComponentId ComponentId, FCommandIndex CommandIndex)
{
	return CommandRequest(OwningCommandRequestPtr(Schema_CopyCommandRequest(Data)), ComponentId, CommandIndex);
}

CommandRequest CommandRequest::DeepCopy() const
{
	check(Data.IsValid());
	return CreateCopy(Data.Get(), ComponentId, CommandIndex);
}

Schema_CommandRequest* CommandRequest::Release() &&
{
	check(Data.IsValid());
	return Data.Release();
}

Schema_Object* CommandRequest::GetRequestObject() const
{
	check(Data.IsValid());
	return Schema_GetCommandRequestObject(Data.Get());
}

Schema_CommandRequest* CommandRequest::GetUnderlying() const
{
	check(Data.IsValid());
	return Data.Get();
}

FComponentId CommandRequest::GetComponentId() const
{
	return ComponentId;
}

FCommandIndex CommandRequest::GetCommandIndex() const
{
	return CommandIndex;
}

} // namespace SpatialGDK
