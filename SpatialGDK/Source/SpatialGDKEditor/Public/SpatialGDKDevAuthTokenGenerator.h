// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Templates/Atomic.h"
#include "Widgets/Notifications/SNotificationList.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKDevAuthTokenGenerator, Log, All);

class SPATIALGDKEDITOR_API FSpatialGDKDevAuthTokenGenerator : public TSharedFromThis<FSpatialGDKDevAuthTokenGenerator>
{
public:
	FSpatialGDKDevAuthTokenGenerator();

	void AsyncGenerateDevAuthToken();

private:
	void ShowTaskStartedNotification(const FString& NotificationText);
	void ShowTaskEndedNotification(const FString& NotificationText, SNotificationItem::ECompletionState CompletionState);

	void EndTask(bool bSuccess);
	void DoGenerateDevAuthTokenTasks();

private:
	TAtomic<bool> bIsGenerating;
	TWeakPtr<SNotificationItem> TaskNotificationPtr;
};
