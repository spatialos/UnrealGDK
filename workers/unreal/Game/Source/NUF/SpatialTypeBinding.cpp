// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTypeBinding.h"
#include "SpatialPackageMapClient.h"

void USpatialTypeBinding::Init(USpatialUpdateInterop* InUpdateInterop, USpatialPackageMapClient* InPackageMap)
{
	check(InUpdateInterop);
	check(InPackageMap);
	UpdateInterop = InUpdateInterop;
	PackageMap = InPackageMap;
}

void USpatialTypeBinding::ResolvePendingRPCs(UObject* Object)
{
	TArray<FCommandRequestContext::FRequestFunction>* RPCList = PendingRPCs.Find(Object);
	if (RPCList)
	{
		for (auto& RequestFunc : *RPCList)
		{
			// We can guarantee that SendCommandRequest won't populate PendingRPCs[Actor], because Actor has
			// been resolved when we call ResolvePendingRPCs.
			SendCommandRequest(RequestFunc);
		}
		PendingRPCs.Remove(Object);
	}
}

void USpatialTypeBinding::SendCommandRequest(FCommandRequestContext::FRequestFunction Function)
{
	// Attempt to trigger command request.
	auto Result = Function();
	if (Result.UnresolvedObject != nullptr)
	{
		// Add to pending RPCs if any actors were unresolved.
		PendingRPCs.FindOrAdd(Result.UnresolvedObject).Add(Function);
	}
	else
	{
		// Add to outgoing RPCs.
		FCommandRequestContext Context{Function};
		OutgoingRPCs.Emplace(Result.RequestId, std::move(Context));
	}
}
