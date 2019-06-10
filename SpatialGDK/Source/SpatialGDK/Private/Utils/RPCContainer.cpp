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
	if(RPCs[int(RPCType::Invalid)].Num() > 0)
	{
		UE_LOG(LogRPCContainer, Error, TEXT("Some RPCs have not been sent"));
	}
}

FRPCMap& RPCContainer::operator[](ESchemaComponentType ComponentType)
{
	switch (ComponentType)
	{
	case SCHEMA_ClientUnreliableRPC:
	case SCHEMA_ServerUnreliableRPC:
	{
		return RPCs[int(RPCType::Unreliable)];
	}
	case SCHEMA_ClientReliableRPC:
	case SCHEMA_ServerReliableRPC:
	{
		return RPCs[int(RPCType::Reliable)];
	}
	case SCHEMA_CrossServerRPC:
	{
		return RPCs[int(RPCType::Commands)];
	}
	case SCHEMA_NetMulticastRPC:
	{
		return RPCs[int(RPCType::Multicast)];
	}
	default:
	{
		checkNoEntry();
		return RPCs[int(RPCType::Invalid)];
		break;
	}
	}
}

const FRPCMap& RPCContainer::operator[](ESchemaComponentType ComponentType) const
{
	return (*(const_cast<RPCContainer*>(this)))[ComponentType];
}

const FRPCMap* RPCContainer::begin() const
{
	return const_cast<RPCContainer*>(this)->begin();
}

const FRPCMap* RPCContainer::end() const
{
	return const_cast<RPCContainer*>(this)->end();
}

FRPCMap* RPCContainer::begin()
{
	return &RPCs[0];
}

FRPCMap* RPCContainer::end()
{
	return &RPCs[int(RPCType::Invalid)];
}

} // namespace SpatialGDK
