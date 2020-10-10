// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/CommonTypes.h"

#include "Templates/UniquePtr.h"
#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct CommandResponseDeleter
{
	void operator()(Schema_CommandResponse* CommandResponse) const noexcept
	{
		if (CommandResponse != nullptr)
		{
			Schema_DestroyCommandResponse(CommandResponse);
		}
	}
};

using OwningCommandResponsePtr = TUniquePtr<Schema_CommandResponse, CommandResponseDeleter>;

// An RAII wrapper for component data.
class CommandResponse
{
public:
	// Creates a new component data.
	explicit CommandResponse(FComponentId ComponentId, FCommandIndex CommandIndex);
	// Takes ownership of component data.
	explicit CommandResponse(OwningCommandResponsePtr Data, FComponentId ComponentId, FCommandIndex CommandIndex);

	~CommandResponse() = default;

	// Moveable, not copyable.
	CommandResponse(const CommandResponse&) = delete;
	CommandResponse(CommandResponse&&) = default;
	CommandResponse& operator=(const CommandResponse&) = delete;
	CommandResponse& operator=(CommandResponse&&) = default;

	static CommandResponse CreateCopy(const Schema_CommandResponse* Data, FComponentId ComponentId, FCommandIndex CommandIndex);

	// Creates a copy of the command response.
	CommandResponse DeepCopy() const;
	// Releases ownership of the command response.
	Schema_CommandResponse* Release() &&;

	Schema_Object* GetResponseObject() const;

	Schema_CommandResponse* GetUnderlying() const;

	FComponentId GetComponentId() const;
	FCommandIndex GetCommandIndex() const;

private:
	FComponentId ComponentId;
	FCommandIndex CommandIndex;
	OwningCommandResponsePtr Data;
};

} // namespace SpatialGDK
