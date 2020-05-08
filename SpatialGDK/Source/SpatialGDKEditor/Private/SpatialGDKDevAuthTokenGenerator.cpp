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

void FSpatialGDKDevAuthTokenGenerator::GenerateDevAuthToken()
{
	if (!bIsGenerating)
	{
		bIsGenerating = true;
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
			{
				AsyncTask(ENamedThreads::GameThread, [this]()
					{
						this->ShowTaskStartedNotification(TEXT("Generating Developer Authentication Token"));
					});
				FString DevAuthToken;
				USpatialGDKSettings* GDKRuntimeSettings = GetMutableDefault<USpatialGDKSettings>();
				bool bSuccess = SpatialCommandUtils::GenerateDevAuthToken(GDKRuntimeSettings->IsRunningInChina(), DevAuthToken);

				USpatialGDKEditorSettings* GDKEditorSettings = GetMutableDefault<USpatialGDKEditorSettings>();
				GDKEditorSettings->DevelopmentAuthenticationToken = DevAuthToken;
				GDKEditorSettings->SaveConfig();
				GDKEditorSettings->SetRuntimeDevelopmentAuthenticationToken();

				// Ensure we enable bUseDevelopmentAuthenticationFlow when using cloud deployment flow.
				GDKRuntimeSettings->bUseDevelopmentAuthenticationFlow = true;
				GDKRuntimeSettings->DevelopmentAuthenticationToken = DevAuthToken;

				if (bSuccess)
				{
					AsyncTask(ENamedThreads::GameThread, [this]()
						{
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

				bIsGenerating = false;
			});

	}
}

void FSpatialGDKDevAuthTokenGenerator::ShowTaskStartedNotification(const FString& NotificationText)
{
	FNotificationInfo Info(FText::AsCultureInvariant(NotificationText));
	//Info.ButtonDetails.Add(
	//	FNotificationButtonInfo(
	//		LOCTEXT("DevAuthTaskCancel", "Cancel"),
	//		LOCTEXT("DevAuthTaskCancelToolTip", "Cancels execution of this task."),
	//		FSimpleDelegate::CreateRaw(this, &FSpatialGDKDevAuthTokenGeneration::HandleCancelButtonClicked),
	//		SNotificationItem::CS_Pending
	//	)
	//);
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
}

#define LOCTEXT_NAMESPACE "SpatialGDKEditorPackageAssembly"
