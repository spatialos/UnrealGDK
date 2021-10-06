// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Templates/UniquePtr.h"
#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct CommandRequestDeleter
{
	void operator()(Schema_CommandRequest* CommandRequest) const noexcept
	{
		if (CommandRequest != nullptr)
		{
			Schema_DestroyCommandRequest(CommandRequest);
		}
	}
};

using OwningCommandRequestPtr = TUniquePtr<Schema_CommandRequest, CommandRequestDeleter>;

// An RAII wrapper for component data.
class CommandRequest
{
public:
	// Creates a new component data.
	explicit CommandRequest(Worker_ComponentId ComponentId, Worker_CommandIndex CommandIndex);
	// Takes ownership of component data.
	explicit CommandRequest(OwningCommandRequestPtr Data, Worker_ComponentId ComponentId, Worker_CommandIndex CommandIndex);

	~CommandRequest() = default;

	// Moveable, not copyable.
	CommandRequest(const CommandRequest&) = delete;
	CommandRequest(CommandRequest&&) = default;
	CommandRequest& operator=(const CommandRequest&) = delete;
	CommandRequest& operator=(CommandRequest&&) = default;

	static CommandRequest CreateCopy(const Schema_CommandRequest* Data, Worker_ComponentId ComponentId, Worker_CommandIndex CommandIndex);

	// Creates a copy of the command request.
	CommandRequest DeepCopy() const;
	// Releases ownership of the command request.
	Schema_CommandRequest* Release() &&;

	Schema_Object* GetRequestObject() const;

	Schema_CommandRequest* GetUnderlying() const;

	Worker_ComponentId GetComponentId() const;
	Worker_CommandIndex GetCommandIndex() const;

private:
	Worker_ComponentId ComponentId;
	Worker_CommandIndex CommandIndex;
	OwningCommandRequestPtr Data;
};

} // namespace SpatialGDK
