// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FSpatialGDKEditorLayoutDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	void ForceRefreshLayout();
private:
	IDetailLayoutBuilder* CurrentLayout = nullptr;
};
