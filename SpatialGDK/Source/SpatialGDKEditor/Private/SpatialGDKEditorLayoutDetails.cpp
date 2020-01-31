// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorLayoutDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "Serialization/JsonSerializer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorLayoutDetails);

TSharedRef<IDetailCustomization> FSpatialGDKEditorLayoutDetails::MakeInstance()
{
	return MakeShareable(new FSpatialGDKEditorLayoutDetails);
}

void FSpatialGDKEditorLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("Cloud Connection");
	CustomCategory.AddCustomRow(FText::FromString("Generate Development Authentication Token"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MaxDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.OnClicked(this, &FSpatialGDKEditorLayoutDetails::ClickedOnButton)
			.Content()
			[
				SNew(STextBlock).Text(FText::FromString("Generate Dev Auth Token"))
			]
		];
}

FReply FSpatialGDKEditorLayoutDetails::ClickedOnButton()
{
	FString CreateDevAuthTokenResult;
	int32 ExitCode;
	FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, TEXT("project auth dev-auth-token create --description=\"Unreal GDK Token\" --json_output"), SpatialGDKServicesConstants::SpatialOSDirectory, CreateDevAuthTokenResult, ExitCode);

	if (ExitCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Warning, TEXT("Unable to generate a development authentication token. %s"), *CreateDevAuthTokenResult);
	};

	FString AuthResult;
	FString DevAuthTokenResult;
	if (!CreateDevAuthTokenResult.Split(TEXT("\n"), &AuthResult, &DevAuthTokenResult) || DevAuthTokenResult.IsEmpty())
	{
		// This is necessary because depending on whether you are already authenticated against spatial, it will either return two json structs or one.
		DevAuthTokenResult = CreateDevAuthTokenResult;
	}

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(DevAuthTokenResult);
	TSharedPtr<FJsonObject> JsonRootObject;
	if (!(FJsonSerializer::Deserialize(JsonReader, JsonRootObject) && JsonRootObject.IsValid()))
	{
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Warning, TEXT("Unable to parse the received development authentication token. %s"), *DevAuthTokenResult);
		return FReply::Handled();
	}

	TSharedPtr<FJsonObject> JsonDataObject = JsonRootObject->GetObjectField("json_data");
	FString TokenSecret = JsonDataObject->GetStringField("token_secret");

	if (USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>())
	{
		SpatialGDKEditorSettings->DevelopmentAuthenticationToken = TokenSecret;
		SpatialGDKEditorSettings->SaveConfig();
		SpatialGDKEditorSettings->SetRuntimeDevelopmentAuthenticationToken();
	}

	return FReply::Handled();
}
