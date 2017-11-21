// Fill out your copyright notice in the Description page of Project Settings.

#include "PackageMapperUtility.h"
#include <string>
#include "Misc/SecureHash.h"
#include "EngineUtils.h"
#include "GenerateSchemaCommandlet.h"

const int StaticObjectOffset = 0x80000000;

void APackageMapperUtility::MapActorPaths(TMap<uint32, FString>& OutMap, UObject* WorldContextObject)
{
	int ActorIndex = StaticObjectOffset;
	for (TActorIterator<AActor> Itr(WorldContextObject->GetWorld()); Itr; ++Itr)
	{
		AActor* Actor = *Itr;
		FStringAssetReference StringRef(Actor);
		FString ActorPath = *StringRef.ToString();
		FString PathHash = *FMD5::HashAnsiString(*Actor->GetPathName());
		uint32 IntPathHash = std::strtoul(TCHAR_TO_UTF8(*PathHash), NULL, 10);
		OutMap.Emplace(IntPathHash/* + StaticObjectOffset*/, ActorPath);
	}
}

void APackageMapperUtility::GeneratePackageMap(UObject* WorldContextObject)
{
	CodeWriter OutputMap;

	TMap<uint32, FString> ObjectPathMap;
	MapActorPaths(ObjectPathMap, WorldContextObject);

	for (auto MapEntry = ObjectPathMap.CreateConstIterator(); MapEntry; ++MapEntry)
	{
		OutputMap.Print(FString::Printf(TEXT("ObjectMap.emplace(%i, \"%s\");"), MapEntry.Key(), *MapEntry.Value()));
	}
	OutputMap.Dump();
		
}
