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
	USpatialGDKEditorSettings* GDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	GDKEditorSettings->DevelopmentAuthenticationToken = DevAuthToken;
	GDKEditorSettings->SaveConfig();
	GDKEditorSettings->SetRuntimeDevelopmentAuthenticationToken();
	GDKEditorSettings->SetRuntimeUseDevelopmentAuthenticationFlow();
}

void FSpatialGDKDevAuthTokenGenerator::DoGenerateDevAuthTokenTasks()
{
	bool bIsRunningInChina = GetDefault<USpatialGDKSettings>()->IsRunningInChina();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, bIsRunningInChina]
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			this->ShowTaskStartedNotification(TEXT("Generating Development Authentication Token"));
		});

		FString DevAuthToken;
		FString ErrorMessage;
		if (SpatialCommandUtils::GenerateDevAuthToken(bIsRunningInChina, DevAuthToken, ErrorMessage))
		{
			AsyncTask(ENamedThreads::GameThread, [this, DevAuthToken]()
			{
				UpdateSettings(DevAuthToken);
				this->ShowTaskEndedNotification(TEXT("Development Authentication Token Updated"), SNotificationItem::CS_Success);
			});
		}
		else
		{
			UE_LOG(LogSpatialGDKDevAuthTokenGenerator, Error, TEXT("Failed to generate a Development Authentication Token:%s"), *ErrorMessage);
			AsyncTask(ENamedThreads::GameThread, [this,&ErrorMessage]()
			{
				this->ShowTaskEndedNotification(FString::Printf(TEXT("Failed to generate Development Authentication Token:%s"), *ErrorMessage), SNotificationItem::CS_Fail);
			});
		}
	});
}

void FSpatialGDKDevAuthTokenGenerator::AsyncGenerateDevAuthToken()
{
	bool bExpected = false;
	if (bIsGenerating.CompareExchange(bExpected, true))
	{
		DoGenerateDevAuthTokenTasks();
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
