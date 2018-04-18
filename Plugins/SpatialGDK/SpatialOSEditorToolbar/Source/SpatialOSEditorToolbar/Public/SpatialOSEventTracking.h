#pragma once

#include "CoreMinimal.h"

UENUM()
enum class ESpatialOSEventTrackingType : uint8
{
  Launch,
  Stop,
  Inspector,
  SettingsSaved
};

UENUM()
enum class ESpatialEventOrigin : uint8
{
  Toolbar,
  Menu
};

class FSpatialOSEventTracking
{
public:
  ~FSpatialOSEventTracking();

  static void LogEvent(ESpatialOSEventTrackingType Type, ESpatialEventOrigin EventOrigin,
                       const FString& AdditionalData = FString());

private:
  FSpatialOSEventTracking();
  static FSpatialOSEventTracking& GetInstance();
  void Initialise();
  void LogEvent_Internal(ESpatialOSEventTrackingType Type, ESpatialEventOrigin EventOrigin,
                         const FString& AdditionalData) const;
  void WriteEventToFile(TSharedPtr<FJsonObject> JsonObject) const;
  FString GenerateProtobufTimestamp() const;

private:
  bool bInitialised;

  // data parsed from project spatialos.json
  FString ProjectName;
  FString ProjectVersion;
  FString SdkVersion;

  // target directory to write event files
  FString EventTargetDirectory;
};
