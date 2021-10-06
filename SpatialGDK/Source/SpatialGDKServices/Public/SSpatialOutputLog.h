// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Developer/OutputLog/Private/SOutputLog.h"
#include "HAL/FileManagerGeneric.h"
#include "IDirectoryWatcher.h"
#include "SlateFwd.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOutputLog, Log, All);

class SSpatialOutputLog : public SOutputLog
{
public:
	SLATE_BEGIN_ARGS(SSpatialOutputLog) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void FormatAndPrintRawLogLine(const FString& LogLine);
	void FormatAndPrintRawErrorLine(const FString& LogLine);

private:
	void OnClearLog() override;
};
