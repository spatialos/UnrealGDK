// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorLayoutDetails, Log, All);

class FSpatialGDKEditorLayoutDetails : public IDetailCustomization
{
private:
	FReply ClickedOnButton();

public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
