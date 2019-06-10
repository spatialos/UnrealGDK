// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RPCContainer.h"

DEFINE_LOG_CATEGORY(LogRPCContainer);

namespace SpatialGDK
{

FPendingRPCParams::FPendingRPCParams(UObject* InTargetObject, UFunction* InFunction, void* InParameters, int InRetryIndex)
	: TargetObject(InTargetObject)
	, Function(InFunction)
	, RetryIndex(InRetryIndex)
	, ReliableRPCIndex(0)
{
	Parameters.SetNumZeroed(Function->ParmsSize);

	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->InitializeValue_InContainer(Parameters.GetData());
		It->CopyCompleteValue_InContainer(Parameters.GetData(), InParameters);
	}
}

FPendingRPCParams::~FPendingRPCParams()
{
	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parameters.GetData());
	}
}

RPCContainer::~RPCContainer()
{
	if(OutgoingRPCs[int(RPCType::Invalid)].Num() > 0)
	{
		UE_LOG(LogRPCContainer, Error, TEXT("Some RPCs have not been sent"));
	}
}

FOutgoingRPCMap& RPCContainer::operator[](ESchemaComponentType ComponentType)
{
	switch (ComponentType)
	{
	case SCHEMA_ClientUnreliableRPC:
	case SCHEMA_ServerUnreliableRPC:
	{
		return OutgoingRPCs[int(RPCType::Unreliable)];
	}
	case SCHEMA_ClientReliableRPC:
	case SCHEMA_ServerReliableRPC:
	{
		return OutgoingRPCs[int(RPCType::Reliable)];
	}
	case SCHEMA_CrossServerRPC:
	{
		return OutgoingRPCs[int(RPCType::Commands)];
	}
	case SCHEMA_NetMulticastRPC:
	{
		return OutgoingRPCs[int(RPCType::Multicast)];
	}
	default:
	{
		checkNoEntry();
		return OutgoingRPCs[int(RPCType::Invalid)];
		break;
	}
	}
}

const FOutgoingRPCMap& RPCContainer::operator[](ESchemaComponentType ComponentType) const
{
	return (*(const_cast<RPCContainer*>(this)))[ComponentType];
}

const FOutgoingRPCMap* RPCContainer::begin() const
{
	return const_cast<RPCContainer*>(this)->begin();
}

const FOutgoingRPCMap* RPCContainer::end() const
{
	return const_cast<RPCContainer*>(this)->end();
}

FOutgoingRPCMap* RPCContainer::begin()
{
	return &OutgoingRPCs[0];
}

FOutgoingRPCMap* RPCContainer::end()
{
	return &OutgoingRPCs[int(RPCType::Invalid)];
}

} // namespace SpatialGDK
