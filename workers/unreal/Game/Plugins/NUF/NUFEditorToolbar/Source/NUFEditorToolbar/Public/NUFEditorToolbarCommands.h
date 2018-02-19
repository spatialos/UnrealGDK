// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "NUFEditorToolbarStyle.h"

class FNUFEditorToolbarCommands : public TCommands<FNUFEditorToolbarCommands>
{
public:
	FNUFEditorToolbarCommands()
		: TCommands<FNUFEditorToolbarCommands>(
			TEXT("NUFEditorToolbar"),
			NSLOCTEXT("Contexts", "NUFEditorToolbar", "NUFEditorToolbar Plugin"), NAME_None,
			FNUFEditorToolbarStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> CreateNUFSnapshot;
};
