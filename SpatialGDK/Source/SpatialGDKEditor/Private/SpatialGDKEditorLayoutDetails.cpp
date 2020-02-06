// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorLayoutDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "HAL/PlatformFilemanager.h"
#include "IOSRuntimeSettings.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Serialization/JsonSerializer.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorLayoutDetails);

TSharedRef<IDetailCustomization> FSpatialGDKEditorLayoutDetails::MakeInstance()
{
	return MakeShareable(new FSpatialGDKEditorLayoutDetails);
}

void FSpatialGDKEditorLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CloudConnectionCategory = DetailBuilder.EditCategory("Cloud Connection");
	CloudConnectionCategory.AddCustomRow(FText::FromString("Generate Development Authentication Token"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.OnClicked(this, &FSpatialGDKEditorLayoutDetails::GenerateDevAuthToken)
			.Content()
			[
				SNew(STextBlock).Text(FText::FromString("Generate Dev Auth Token"))
			]
		];

	IDetailCategoryBuilder& MobileCategory = DetailBuilder.EditCategory("Mobile");
	MobileCategory.AddCustomRow(FText::FromString("Push SpatialOS settings to Android device"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.OnClicked(this, &FSpatialGDKEditorLayoutDetails::PrepareAndroidApplication)
		.Content()
		[
			SNew(STextBlock).Text(FText::FromString("Push SpatialOS settings to Android device"))
		]
		];

	MobileCategory.AddCustomRow(FText::FromString("Push SpatialOS settings to iOS device"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
		.OnClicked(this, &FSpatialGDKEditorLayoutDetails::PrepareIOSApplication)
		.Content()
		[
			SNew(STextBlock).Text(FText::FromString("Push SpatialOS settings to iOS device"))
		]
		];
}

FReply FSpatialGDKEditorLayoutDetails::GenerateDevAuthToken()
{
	FString CreateDevAuthTokenResult;
	int32 ExitCode;
	FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, TEXT("project auth dev-auth-token create --description=\"Unreal GDK Token\" --json_output"), SpatialGDKServicesConstants::SpatialOSDirectory, CreateDevAuthTokenResult, ExitCode);

	if (ExitCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Error, TEXT("Unable to generate a development authentication token. Result: %s"), *CreateDevAuthTokenResult);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Unable to generate a development authentication token. Result: %s"), *CreateDevAuthTokenResult)));
		return FReply::Unhandled();
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
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Error, TEXT("Unable to parse the received development authentication token. Result: %s"), *DevAuthTokenResult);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Unable to parse the received development authentication token. Result: %s"), *DevAuthTokenResult)));
		return FReply::Unhandled();
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

bool FSpatialGDKEditorLayoutDetails::TryConstructMobileCommandLineArgumentsFile(FString& CommandLineArgsFile)
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	const FString ProjectName = FApp::GetProjectName();

	// The project path is based on this: https://github.com/improbableio/UnrealEngine/blob/4.22-SpatialOSUnrealGDK-release/Engine/Source/Programs/AutomationTool/AutomationUtils/DeploymentContext.cs#L408
	const FString IOSProjectPath = FString::Printf(TEXT("../../../%s/%s.uproject"), *ProjectName, *ProjectName);
	FString TravelUrl;
	FString SpatialOSCommandLineArgs = FString::Printf(TEXT("-workerType %s"), *(SpatialGDKSettings->MobileWorkerType));
	if (SpatialGDKSettings->bMobileConnectToLocalDeployment)
	{
		if (SpatialGDKSettings->MobileRuntimeIP.IsEmpty())
		{
			UE_LOG(LogSpatialGDKEditorLayoutDetails, Error, TEXT("The Runtime IP is currently not set. Please make sure to specify a Runtime IP."));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("The Runtime IP is currently not set. Please make sure to specify a Runtime IP."))));
			return false;
		}

		TravelUrl = SpatialGDKSettings->MobileRuntimeIP;
	}
	else
	{
		TravelUrl = TEXT("connect.to.spatialos");

		if (SpatialGDKSettings->DevelopmentAuthenticationToken.IsEmpty())
		{
			FReply GeneratedToken = GenerateDevAuthToken();
			if (!GeneratedToken.IsEventHandled())
			{
				return false;
			}
		}

		SpatialOSCommandLineArgs += FString::Printf(TEXT(" +devauthToken %s"), *(SpatialGDKSettings->DevelopmentAuthenticationToken));
		if (!SpatialGDKSettings->DevelopmentDeploymentToConnect.IsEmpty())
		{
			SpatialOSCommandLineArgs += FString::Printf(TEXT(" +deployment %s"), *(SpatialGDKSettings->DevelopmentDeploymentToConnect));
		}
	}

	const FString FinalCommandLineArgs = FString::Printf(TEXT("%s %s %s %s"), *IOSProjectPath, *TravelUrl, *SpatialOSCommandLineArgs, *(SpatialGDKSettings->MobileExtraCommandLineArgs));
	CommandLineArgsFile = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::ProjectLogDir(), TEXT("ue4commandline.txt")));

	if (!FFileHelper::SaveStringToFile(FinalCommandLineArgs, *CommandLineArgsFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Error, TEXT("Failed to write command line args to file: %s"), *CommandLineArgsFile);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Failed to write command line args to file: %s"), *CommandLineArgsFile)));
		return false;
	}

	return true;
}

bool FSpatialGDKEditorLayoutDetails::TryPushCommandLineArgs(const FString& Executable, const FString& ExeArguments, const FString& CommandLineArgsFile)
{
	FString OutCommandLineArgsFile;
	FString ExeOutput;
	FString StdErr;
	int32 ExitCode;

	FPlatformProcess::ExecProcess(*Executable, *ExeArguments, &ExitCode, &ExeOutput, &StdErr);
	if (ExitCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Error, TEXT("Failed to update the mobile client. %s %s"), *ExeOutput, *StdErr);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to update the mobile client. See the Output log for more information.")));
		return false;
	}

	UE_LOG(LogSpatialGDKEditorLayoutDetails, Log, TEXT("Successfully stored command line args on device: %s"), *ExeOutput);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DeleteFile(*CommandLineArgsFile))
	{
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Error, TEXT("Failed to delete file %s"), *CommandLineArgsFile);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(TEXT("Failed to delete file %s"), *CommandLineArgsFile)));
		return false;
	}

	return true;
}

FReply FSpatialGDKEditorLayoutDetails::PrepareAndroidApplication()
{
	FString AndroidHome = FPlatformMisc::GetEnvironmentVariable(TEXT("ANDROID_HOME"));
	if (AndroidHome.IsEmpty())
	{
		UE_LOG(LogSpatialGDKEditorLayoutDetails, Error, TEXT("Environment variable ANDROID_HOME is not set. Please make sure to configure this."));
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Environment variable ANDROID_HOME is not set. Please make sure to configure this.")));
		return FReply::Unhandled();
	}

	FString OutCommandLineArgsFile;

	if (!TryConstructMobileCommandLineArgumentsFile(OutCommandLineArgsFile))
	{
		return FReply::Unhandled();
	}

	const FString ProjectName = FApp::GetProjectName();
	const FString AndroidCommandLineFile = FString::Printf(TEXT("/mnt/sdcard/UE4Game/%s/UE4CommandLine.txt"), *ProjectName);
	const FString AdbArguments = FString::Printf(TEXT("push %s %s"), *OutCommandLineArgsFile, *AndroidCommandLineFile);
	FString AdbExe = FPaths::ConvertRelativePathToFull(FPaths::Combine(AndroidHome, TEXT("platform-tools/adb")));

#if PLATFORM_WINDOWS
	AdbExe = FString::Printf(TEXT("%s.exe"), *AdbExe);
#endif

	TryPushCommandLineArgs(AdbExe, AdbArguments, OutCommandLineArgsFile);
	return FReply::Handled();
}

FReply FSpatialGDKEditorLayoutDetails::PrepareIOSApplication()
{
	const UIOSRuntimeSettings* IOSRuntimeSettings = GetDefault<UIOSRuntimeSettings>();
	FString OutCommandLineArgsFile;

	if (!TryConstructMobileCommandLineArgumentsFile(OutCommandLineArgsFile))
	{
		return FReply::Unhandled();
	}

	FString Executable = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/DotNET/IOS/deploymentserver.exe")));
	FString DeploymentServerArguments = FString::Printf(TEXT("copyfile -bundle \"%s\" -file \"%s\" -file \"/Documents/ue4commandline.txt\""), *(IOSRuntimeSettings->BundleIdentifier), *OutCommandLineArgsFile);

#if PLATFORM_MAC
	DeploymentServerArguments = FString::Printf(TEXT("%s %s"), *Executable, *DeploymentServerArguments);
	Executable = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/ThirdParty/Mono/Mac/bin/mono")));
#endif

	TryPushCommandLineArgs(Executable, DeploymentServerArguments, OutCommandLineArgsFile);
	return FReply::Handled();
}
