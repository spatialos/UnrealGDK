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

void FSpatialGDKDevAuthTokenGenerator::DoGenerateDevAuthTokenTasks()
{
	bool bIsRunningInChina = GetDefault<USpatialGDKSettings>()->IsRunningInChina();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, bIsRunningInChina] {
		AsyncTask(ENamedThreads::GameThread, [this]() {
			ShowTaskStartedNotification(TEXT("Generating Development Authentication Token"));
		});

		FString DevAuthToken;
		FText ErrorMessage;
		if (SpatialCommandUtils::GenerateDevAuthToken(bIsRunningInChina, DevAuthToken, ErrorMessage))
		{
			AsyncTask(ENamedThreads::GameThread, [this, DevAuthToken]() {
				GetMutableDefault<USpatialGDKEditorSettings>()->SetDevelopmentAuthenticationToken(DevAuthToken);
				EndTask(/* bSuccess */ true);
			});
		}
		else
		{
			UE_LOG(LogSpatialGDKDevAuthTokenGenerator, Error, TEXT("Failed to generate a Development Authentication Token: %s"),
				   *ErrorMessage.ToString());
			AsyncTask(ENamedThreads::GameThread, [this]() {
				EndTask(/* bSuccess */ false);
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
		UE_LOG(LogSpatialGDKDevAuthTokenGenerator, Display,
			   TEXT("A previous Development Authentication Token request is still pending. New request for generation ignored."));
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

void FSpatialGDKDevAuthTokenGenerator::EndTask(bool bSuccess)
{
	if (bSuccess)
	{
		ShowTaskEndedNotification(TEXT("Development Authentication Token Updated"), SNotificationItem::CS_Success);
	}
	else
	{
		ShowTaskEndedNotification(TEXT("Failed to generate Development Authentication Token"), SNotificationItem::CS_Fail);
	}

	bIsGenerating = false;
}

void FSpatialGDKDevAuthTokenGenerator::ShowTaskEndedNotification(const FString& NotificationText,
																 SNotificationItem::ECompletionState CompletionState)
{
	TSharedPtr<SNotificationItem> Notification = TaskNotificationPtr.Pin();
	if (Notification.IsValid())
	{
		Notification->SetFadeInDuration(0.1f);
		Notification->SetFadeOutDuration(0.5f);
		Notification->SetExpireDuration(5.0f);
		Notification->SetText(FText::AsCultureInvariant(NotificationText));
		Notification->SetCompletionState(CompletionState);
		Notification->ExpireAndFadeout();
	}
}
