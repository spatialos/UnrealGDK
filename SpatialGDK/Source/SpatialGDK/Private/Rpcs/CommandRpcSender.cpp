// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Rpcs/CommandRpcSender.h"

namespace SpatialGDK
{
FCommandRpcSender::FCommandRpcSender(SpatialInterface* Sender, FCommandRpcWriter RpcWriter)
	: RpcWriter(RpcWriter)
	, Sender(Sender)
{
}
void FCommandRpcSender::SendAndFlush(Worker_EntityId EntityId, RPCPayload Rpc)
{
	Sender->SendCommand(EntityId, RpcWriter.CreateRequest(MoveTemp(Rpc)));
}

} // namespace SpatialGDK
