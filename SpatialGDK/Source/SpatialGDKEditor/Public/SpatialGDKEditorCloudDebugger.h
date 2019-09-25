// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once
#include "Containers/UnrealString.h"
#include "ISessionFrontendModule.h"
#include "ITcpMessagingModule.h"

class FSpatialGDKEditorCloudDebugger
{
public:
	FSpatialGDKEditorCloudDebugger(); 
	~FSpatialGDKEditorCloudDebugger();

	void DebugWorker(const FString& InDeploymentName, const FString& InWorkerId);

private:
	void ForceSpatialLogin();
	void ClosePortForward();

	ISessionFrontendModule* SessionFrontendModule;
	ITcpMessagingModule* TcpMessagingModule;
	FProcHandle PortForwardHandle;
};
