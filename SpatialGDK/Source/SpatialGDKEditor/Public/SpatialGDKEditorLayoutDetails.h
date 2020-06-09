// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IErrorReportingWidget;

class FSpatialGDKEditorLayoutDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	IDetailLayoutBuilder* CurrentLayout = nullptr;
	TSharedPtr<IErrorReportingWidget> ProjectNameInputErrorReporting;

	/** Delegate to commit project name */
	void OnProjectNameCommitted(const FText& InText, ETextCommit::Type InCommitType);
};
