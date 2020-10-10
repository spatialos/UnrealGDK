#include "SpatialView/CommandResponse.h"

namespace SpatialGDK
{
CommandResponse::CommandResponse(FComponentId ComponentId, FCommandIndex CommandIndex)
	: ComponentId(ComponentId)
	, CommandIndex(CommandIndex)
	, Data(Schema_CreateCommandResponse())
{
}

CommandResponse::CommandResponse(OwningCommandResponsePtr Data, FComponentId ComponentId, FCommandIndex CommandIndex)
	: ComponentId(ComponentId)
	, CommandIndex(CommandIndex)
	, Data(MoveTemp(Data))
{
}

CommandResponse CommandResponse::CreateCopy(const Schema_CommandResponse* Data, FComponentId ComponentId, FCommandIndex CommandIndex)
{
	return CommandResponse(OwningCommandResponsePtr(Schema_CopyCommandResponse(Data)), ComponentId, CommandIndex);
}

CommandResponse CommandResponse::DeepCopy() const
{
	check(Data.IsValid());
	return CreateCopy(Data.Get(), ComponentId, CommandIndex);
}

Schema_CommandResponse* CommandResponse::Release() &&
{
	check(Data.IsValid());
	return Data.Release();
}

Schema_Object* CommandResponse::GetResponseObject() const
{
	check(Data.IsValid());
	return Schema_GetCommandResponseObject(Data.Get());
}

Schema_CommandResponse* CommandResponse::GetUnderlying() const
{
	check(Data.IsValid());
	return Data.Get();
}

FComponentId CommandResponse::GetComponentId() const
{
	return ComponentId;
}

FCommandIndex CommandResponse::GetCommandIndex() const
{
	return CommandIndex;
}

} // namespace SpatialGDK
