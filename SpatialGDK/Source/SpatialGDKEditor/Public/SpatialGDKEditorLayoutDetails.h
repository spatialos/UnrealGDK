// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FReply;
class FSpatialGDKEditorCommandLineArgsManager;

class FSpatialGDKEditorLayoutDetails : public IDetailCustomization
{
private:
	FSpatialGDKEditorCommandLineArgsManager& GetCommandLineArgsManager();

	FReply GenerateDevAuthToken();
	FReply PushCommandLineArgsToIOSDevice();
	FReply PushCommandLineArgsToAndroidDevice();
	FReply RemoveCommandLineArgsFromIOSDevice();
	FReply RemoveCommandLineArgsFromAndroidDevice();


	void ForceRefreshLayout();

	IDetailLayoutBuilder* CurrentLayout = nullptr;

public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
