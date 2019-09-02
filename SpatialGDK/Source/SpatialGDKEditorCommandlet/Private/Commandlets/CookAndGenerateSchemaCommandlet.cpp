// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "CookAndGenerateSchemaCommandlet.h"
#include "SpatialGDKEditorCommandletPrivate.h"
#include "SpatialGDKEditorSchemaGenerator.h"
#include "Async.h"
#include "AssetRegistryModule.h"
#include "ModuleManager.h"
#include "AssetData.h"
#include "AssetRegistry/Public/AssetDataTagMap.h"
#include "Engine/Blueprint.h"
#include "Engine/LevelScriptActor.h"

struct FObjectListener : public FUObjectArray::FUObjectCreateListener
{
	FObjectListener()
	{
		GUObjectArray.AddUObjectCreateListener(this);
	}

	~FObjectListener()
	{
	}

	virtual void NotifyUObjectCreated(const class UObjectBase *Object, int32 Index) override
	{
		if (!VisitedClasses.Contains(Object->GetClass()) && IsSupportedClass(Object->GetClass()))
		{
			UE_LOG(LogTemp, Display, TEXT("[Object Created] Path: %s Class: %s"), *Object->GetFName().ToString(), *GetPathNameSafe(Object->GetClass()));
			VisitedClasses.Add(FSoftClassPath(Object->GetClass()));
		}
	}

public:
	TSet<FSoftClassPath> VisitedClasses;
};

UCookAndGenerateSchemaCommandlet::UCookAndGenerateSchemaCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;

	ObjectListener = new FObjectListener();
}

UCookAndGenerateSchemaCommandlet::~UCookAndGenerateSchemaCommandlet()
{
	delete ObjectListener;
	ObjectListener = nullptr;
}

bool UCookAndGenerateSchemaCommandlet::IsEditorOnly() const
{
	return true;
}

int32 UCookAndGenerateSchemaCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Cook and Generate Schema Started."));
	//GEditor->OnObjectsReplaced().AddUObject(this, &UCookAndGenerateSchemaCommandlet::OnObjectsReplaced);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Try Load Schema Database."));
	if (!TryLoadExistingSchemaDatabase())
	{
		return 1;
	}

	//FindAllDataOnlyBlueprints();

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Generate Schema for in-memory Classes."));
	if (!SpatialGDKGenerateSchema(false /* bSaveSchemaDatabase */, false /* bRunSchemaCompiler */))
	{
		return 2;
	}

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Starting Cook Command."));
	int32 CookResult = Super::Main(CmdLineParams);
	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Cook Command Completed."));

	GUObjectArray.RemoveUObjectCreateListener(ObjectListener);

	ReferencedClasses.Append(ObjectListener->VisitedClasses);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Discovered %d Classes during cook."), ReferencedClasses.Num());

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Load/Gen Started at %s."), *FDateTime::Now().ToIso8601());
	TSet<UClass*> Classes;
	const int BatchSize = 100;
	for (FSoftClassPath SoftPath : ReferencedClasses)
	{
		if (!SoftPath.GetAssetPathString().StartsWith(TEXT("/Game"))) continue;
		if (UClass* LoadedClass = SoftPath.TryLoadClass<UObject>())
		{
			UE_LOG(LogTemp, Display, TEXT("Reloaded %s, add to batch"), *GetPathNameSafe(LoadedClass));
			Classes.Add(LoadedClass);
			if (Classes.Num() >= BatchSize)
			{
				SpatialGDKGenerateSchemaForClasses(Classes);
				Classes.Empty(100);
			}
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("Failed to load %s for schema gen"), *SoftPath.ToString());
		}
	}
	SpatialGDKGenerateSchemaForClasses(Classes);

	UE_LOG(LogSpatialGDKEditorCommandlet, Display, TEXT("Schema Load/Gen Finished at %s."), *FDateTime::Now().ToIso8601());
	
	if (!SaveSchemaDatabase())
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Error, TEXT("Failed to save schema database."));
		return 3;
	}

	if (!RunSchemaCompiler())
	{
		UE_LOG(LogSpatialGDKEditorCommandlet, Warning, TEXT("Failed to run schema compiler."));
	}

	return CookResult;
}

//void UCookAndGenerateSchemaCommandlet::OnObjectsReplaced(const TMap<UObject*, UObject*>& ObjectsReplaced)
//{
//	TSet<FSoftClassPath> CandidateClasses;
//	for (auto& Entry : ObjectsReplaced)
//	{
//		FString FromPath = GetPathNameSafe(Entry.Key);
//		FString ToPath = GetPathNameSafe(Entry.Value);
//
//		UE_LOG(LogTemp, Display, TEXT("[%s] => [%s]"), *FromPath, *ToPath);
//
//		if (const UObject* NewObject = Entry.Value)
//		{
//			ReferencedClasses.Add(FSoftClassPath(NewObject->GetClass()));
//			/*if (NewObject->HasAllFlags(RF_ArchetypeObject))
//			{
//				bool bAlreadyLoaded = false;
//				ReferencedClasses.Add(NewObject->GetClass()->GetPathName(), &bAlreadyLoaded);
//				if (!bAlreadyLoaded)
//				{
//					UE_LOG(LogTemp, Display, TEXT("Generate for [%s] Object Flags(%#010x) ClassFlags(%#010x) ClassObjectFlags(%#010x)"), *GetPathNameSafe(NewObject->GetClass()),
//						NewObject->GetFlags(), NewObject->GetClass()->GetClassFlags(), NewObject->GetClass()->GetFlags());
//					CandidateClasses.Add(FSoftClassPath(NewObject->GetClass()));
//				}
//			}
//			else
//			{
//				UE_LOG(LogTemp, Display, TEXT("Ignoring [%s]"), *GetPathNameSafe(NewObject->GetClass()));
//			}*/
//		}
//	}
//	/*if (CandidateClasses.Num() > 0)
//	{
//		TSet<UClass*> ActualClasses;
//		for (FSoftClassPath SoftClass : CandidateClasses)
//		{
//			UE_LOG(LogTemp, Display, TEXT("Loading %s"), *SoftClass.ToString());
//
//			if (UClass* SomeClass = StaticLoadClass(UObject::StaticClass(), nullptr, *SoftClass.ToString()))
//			{
//				UE_LOG(LogTemp, Display, TEXT("Loaded Class %s (%#010x)"), *GetPathNameSafe(SomeClass), SomeClass->GetFlags());
//				if (SomeClass->HasAnyFlags(RF_NeedPostLoadSubobjects))
//				{
//					UE_LOG(LogTemp, Display, TEXT("Subobjects not loaded, skipping %s"), *GetPathNameSafe(SomeClass));
//				}
//				else
//				{
//					ActualClasses.Add(SomeClass);
//				}
//			}
//			else
//			{
//				UE_LOG(LogTemp, Display, TEXT("Failed to load %s"), *SoftClass.ToString());
//			}
//		}
//		if (ActualClasses.Num() > 0)
//		{
//			UE_LOG(LogTemp, Display, TEXT("Submit %d Classes for schema gen:"), ActualClasses.Num());
//			for (UClass* AClass : ActualClasses)
//			{
//				UE_LOG(LogTemp, Display, TEXT("- %s"), *GetPathNameSafe(AClass));
//			}
//			SpatialGDKGenerateSchemaForClasses(ActualClasses);
//		}
//	}*/
//}
//
//void UCookAndGenerateSchemaCommandlet::FindAllDataOnlyBlueprints()
//{
//	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
//	TArray<FAssetData> AssetData;
//	FARFilter Filter;
//	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
//	Filter.TagsAndValues.AddUnique(FBlueprintTags::IsDataOnly, TOptional<FString>(TEXT("True")));
//	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
//
//	UE_LOG(LogTemp, Display, TEXT("Found %d Data-only Blueprints."), AssetData.Num());
//	for (const FAssetData& Asset : AssetData)
//	{
//		FString ParentClassPath = TEXT("None");
//		Asset.GetTagValue(FBlueprintTags::ParentClassPath, ParentClassPath);
//
//		UE_LOG(LogTemp, Display, TEXT("[%s] Parent Class [%s]"), *Asset.ObjectPath.ToString(), *ParentClassPath);
//		Asset.ToSoftObjectPath().TryLoad();
//	}
//}
