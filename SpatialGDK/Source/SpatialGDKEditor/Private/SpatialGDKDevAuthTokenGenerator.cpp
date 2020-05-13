// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKDevAuthTokenGenerator.h"

#include "Async/Async.h"
#include "Framework/Notifications/NotificationManager.h"

#include "SpatialCommandUtils.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKDevAuthTokenGenerator);

FSpatialGDKDevAuthTokenGenerator::FSpatialGDKDevAuthTokenGenerator()
	: bIsGenerating(false)
{
}

void FSpatialGDKDevAuthTokenGenerator::UpdateSettings(FString DevAuthToken)
{
	AsyncTask(ENamedThreads::GameThread, [this, DevAuthToken]()
	{
		USpatialGDKEditorSettings* GDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
		GDKEditorSettings->DevelopmentAuthenticationToken = DevAuthToken;
		GDKEditorSettings->SaveConfig();
		GDKEditorSettings->SetRuntimeDevelopmentAuthenticationToken();
		GDKEditorSettings->SetRuntimeUseDevelopmentAuthenticationFlow();

		this->ShowTaskEndedNotification(TEXT("Development Authentication Token Updated"), SNotificationItem::CS_Success);
	});
}

void FSpatialGDKDevAuthTokenGenerator::DoGenerateDevAuthToken()
{
	bool bIsRunningInChina = GetDefault<USpatialGDKSettings>()->IsRunningInChina();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, bIsRunningInChina]
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			this->ShowTaskStartedNotification(TEXT("Generating Development Authentication Token"));
		});

		FString DevAuthToken;
		if (SpatialCommandUtils::GenerateDevAuthToken(bIsRunningInChina, DevAuthToken))
		{
			UpdateSettings(DevAuthToken);
		}
		else
		{
			UE_LOG(LogSpatialGDKDevAuthTokenGenerator, Error, TEXT("Failed to generate a development authentication token."));
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				this->ShowTaskEndedNotification(TEXT("Failed to generate Development Authentication Token"), SNotificationItem::CS_Fail);
			});
		}
	});
}

void FSpatialGDKDevAuthTokenGenerator::AsyncGenerateDevAuthToken()
{
	bool bExpected = false;
	if (bIsGenerating.CompareExchange(bExpected, true))
	{
		DoGenerateDevAuthToken();
	}
	else
	{
		UE_LOG(LogSpatialGDKDevAuthTokenGenerator, Display, TEXT("A previous Development Authentication Token request is still pending. New request for generation ignored."));
	}
}

void FSpatialGDKDevAuthTokenGenerator::ShowTaskStartedNotification(const FString& NotificationText)
{
	FNotificationInfo Info(FText::AsCultureInvariant(NotificationText));
	Info.ExpireDuration = 5.0f;
	Info.bFireAndForget = false;

	TaskNotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

	if (TaskNotificationPtr.IsValid())
	{
		TaskNotificationPtr.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

void FSpatialGDKDevAuthTokenGenerator::ShowTaskEndedNotification(const FString& NotificationText, SNotificationItem::ECompletionState CompletionState)
{
	TSharedPtr<SNotificationItem> Notification = TaskNotificationPtr.Pin();
	if (Notification.IsValid())
	{
		Notification->SetFadeInDuration(0.1f);
		Notification->SetFadeOutDuration(0.5f);
		Notification->SetExpireDuration(5.0);
		Notification->SetText(FText::AsCultureInvariant(NotificationText));
		Notification->SetCompletionState(CompletionState);
		Notification->ExpireAndFadeout();
	}
	bIsGenerating = false;
}

#define LOCTEXT_NAMESPACE "SpatialGDKEditorPackageAssembly"
