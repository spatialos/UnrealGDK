// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SpatialOSEditorToolbarStyle.h"

class FSpatialOSEditorToolbarCommands : public TCommands<FSpatialOSEditorToolbarCommands>
{
public:
	FSpatialOSEditorToolbarCommands() : TCommands<FSpatialOSEditorToolbarCommands>(TEXT("SpatialOSEditorToolbar"), NSLOCTEXT("Contexts", "SpatialOSEditorToolbar", "SpatialOSEditorToolbar Plugin"), NAME_None, FSpatialOSEditorToolbarStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> StartSpatialOSStackAction;
	TSharedPtr<FUICommandInfo> StopSpatialOSStackAction;
	TSharedPtr<FUICommandInfo> LaunchInspectorWebPageAction;
};
