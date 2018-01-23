// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PackageMapperUtility.h"
#include <string>
#include "Misc/SecureHash.h"
#include "EngineUtils.h"
#include "CodeWriter.h"
#include <inttypes.h>

const uint32 StaticObjectOffset = 0x80000000;

uint32 APackageMapperUtility::Hash(FString& Input)
{
	int Res = 0;
	for (int i = 0; i < Input.Len(); i++)
	{
		Res += (i * Input[i] % 0xFFFFFFFF);
	}
	return Res;
}

void APackageMapperUtility::MapActorPaths(TMap<uint32, FString>& OutMap, UObject* WorldContextObject)
{
	int ActorIndex = StaticObjectOffset;
	for (TActorIterator<AActor> Itr(WorldContextObject->GetWorld()); Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		FStringAssetReference StringRef(Actor);
		FString PathStr = StringRef.ToString();
		std::hash<std::string> Hasher;
		uint32 PathHash = Hasher(TCHAR_TO_UTF8(*PathStr));
		
		OutMap.Emplace((PathHash & (StaticObjectOffset - 1)) + StaticObjectOffset, PathStr);
	}
}

void APackageMapperUtility::GeneratePackageMap(UObject* WorldContextObject)
{
	FCodeWriter OutputMap;

	TMap<uint32, FString> ObjectPathMap;
	MapActorPaths(ObjectPathMap, WorldContextObject);

	for (auto MapEntry = ObjectPathMap.CreateConstIterator(); MapEntry; ++MapEntry)
	{
		OutputMap.Print(FString::Printf(TEXT("ObjectMap.emplace(%" PRIu32 ", \"%s\");"), MapEntry.Key(), *MapEntry.Value()));
	}
	OutputMap.Dump();
}
