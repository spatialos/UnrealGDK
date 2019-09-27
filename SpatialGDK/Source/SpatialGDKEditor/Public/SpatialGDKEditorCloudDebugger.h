// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Containers/UnrealString.h"
#include "EngineServiceMessages.h"
#include "ISessionFrontendModule.h"
#include "ITcpMessagingModule.h"
#include "MessageEndpoint.h"

class FSpatialGDKEditorCloudDebugger
{
public:
	FSpatialGDKEditorCloudDebugger(); 
	~FSpatialGDKEditorCloudDebugger();

	void DebugWorker(const FString& InDeploymentName, const FString& InWorkerId);

private:
	void ForceSpatialLogin();
	void ClosePortForward();

	void HandleServicePongMessage(const FEngineServicePong& Message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& Context);
	bool HandleTicker(float DeltaTime);

	ISessionFrontendModule* SessionFrontendModule;
	ITcpMessagingModule* TcpMessagingModule;
	FProcHandle PortForwardHandle;

	TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> MessageEndpoint;
	FDelegateHandle TickDelegateHandle;
	FDateTime LastPingTime;

	/** Addresses of other endpoints that were tried to get access to execute remote commands.*/
	TArray<FMessageAddress> TriedAddresses;
};
