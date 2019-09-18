// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once
#include "Containers/UnrealString.h"

class FSpatialGDKEditorCloudDebugger
{
public:
	FSpatialGDKEditorCloudDebugger(); 

	void DebugWorker(const FString& InDeploymentName, const FString& InWorkerId);
};
