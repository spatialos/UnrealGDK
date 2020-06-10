// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Templates/SharedPointer.h"
#include "IDetailCustomization.h"

class FLaunchConfigEditorLayoutDetails : public IDetailCustomization
{
private:
	void ForceRefreshLayout();

	IDetailLayoutBuilder* MyLayout = nullptr;

public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
