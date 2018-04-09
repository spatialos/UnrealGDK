// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SpatialGDKEditorToolbarStyle.h"

class FSpatialGDKEditorToolbarCommands : public TCommands<FSpatialGDKEditorToolbarCommands>
{
public:
	FSpatialGDKEditorToolbarCommands()
		: TCommands<FSpatialGDKEditorToolbarCommands>(
			TEXT("SpatialGDKEditorToolbar"),
			NSLOCTEXT("Contexts", "SpatialGDKEditorToolbar", "SpatialGDKEditorToolbar Plugin"), NAME_None,
			FSpatialGDKEditorToolbarStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> CreateSpatialGDKSnapshot;
	TSharedPtr<FUICommandInfo> GenerateInteropCode;
};
