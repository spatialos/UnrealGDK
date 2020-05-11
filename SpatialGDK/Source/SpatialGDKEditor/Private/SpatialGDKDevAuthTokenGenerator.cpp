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

void FSpatialGDKDevAuthTokenGenerator::AsyncGenerateDevAuthToken()
{
	bool bExpected = false;
	if (bIsGenerating.CompareExchange(bExpected, true))
	{
		DevAuthToken.Empty();
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
					{
						this->ShowTaskStartedNotification(TEXT("Generating Developer Authentication Token"));
					});

				USpatialGDKSettings* GDKRuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
				bool bSuccess = SpatialCommandUtils::GenerateDevAuthToken(GDKRuntimeSettings->IsRunningInChina(), DevAuthToken);
				if (bSuccess)
				{
					AsyncTask(ENamedThreads::GameThread, [this]()
						{
							USpatialGDKEditorSettings* GDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
							GDKEditorSettings->DevelopmentAuthenticationToken = DevAuthToken;
							GDKEditorSettings->SaveConfig();
							GDKEditorSettings->SetRuntimeDevelopmentAuthenticationToken();

							// Ensure we enable bUseDevelopmentAuthenticationFlow when using cloud deployment flow.
							USpatialGDKSettings* GDKRuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
							GDKRuntimeSettings->bUseDevelopmentAuthenticationFlow = true;
							GDKRuntimeSettings->DevelopmentAuthenticationToken = DevAuthToken;

							this->ShowTaskEndedNotification(TEXT("Developer Authentication Token Updated"), SNotificationItem::CS_Success);
						});
				}
				else
				{
					UE_LOG(LogSpatialGDKDevAuthTokenGenerator, Error, TEXT("Failed to generate a development authentication token."));
					AsyncTask(ENamedThreads::GameThread, [this]()
						{
							this->ShowTaskEndedNotification(TEXT("Developer Authentication Token Updated"), SNotificationItem::CS_Fail);
						});
				}
			});
	}
	else
	{
		UE_LOG(LogSpatialGDKDevAuthTokenGenerator, Display, TEXT("Developer Authentication Token requested but a previous request is still pending. New request for generation ignored."));
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
