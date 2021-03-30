#include "SpatialView/CommandResponse.h"

namespace SpatialGDK
{
CommandResponse::CommandResponse(Worker_ComponentId ComponentId, Worker_CommandIndex CommandIndex)
	: ComponentId(ComponentId)
	, CommandIndex(CommandIndex)
	, Data(Schema_CreateCommandResponse())
{
}

CommandResponse::CommandResponse(OwningCommandResponsePtr Data, Worker_ComponentId ComponentId, Worker_CommandIndex CommandIndex)
	: ComponentId(ComponentId)
	, CommandIndex(CommandIndex)
	, Data(MoveTemp(Data))
{
}

CommandResponse CommandResponse::CreateCopy(const Schema_CommandResponse* Data, Worker_ComponentId ComponentId,
											Worker_CommandIndex CommandIndex)
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

Worker_ComponentId CommandResponse::GetComponentId() const
{
	return ComponentId;
}

Worker_CommandIndex CommandResponse::GetCommandIndex() const
{
	return CommandIndex;
}

} // namespace SpatialGDK
