// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorInteropCodeGenerator.h"

#include "AssetRegistryModule.h"
#include "Engine/LevelScriptActor.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/MonitoredProcess.h"
#include "SchemaGenerator.h"
#include "SharedPointer.h"
#include "TypeStructure.h"
#include "SpatialConstants.h"
#include "SpatialGDKEditorToolbarSettings.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "Utils/DataTypeUtilities.h"
#include "Utils/SchemaDatabase.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKInteropCodeGenerator);

namespace
{

void OnStatusOutput(FString Message)
{
	UE_LOG(LogSpatialGDKInteropCodeGenerator, Log, TEXT("%s"), *Message);
}

int GenerateCompleteSchemaFromClass(const FString& SchemaPath, int ComponentId, UClass* Class)
{
	FCodeWriter OutputSchema;

	FString SchemaFilename = FString::Printf(TEXT("Unreal%s"), *UnrealNameToSchemaTypeName(Class->GetName()));

	// Parent and static array index start at 0 for checksum calculations.
	TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, 0, 0, false);

	// Generate schema.
	int NumComponents = GenerateTypeBindingSchema(OutputSchema, ComponentId, Class, TypeInfo, SchemaPath);
	OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFilename));

	return NumComponents;
}

bool CheckClassNameListValidity(const TArray<UClass*>& Classes)
{
	// Remove all underscores from the class names, check for duplicates.
	for (int i = 0; i < Classes.Num() - 1; ++i)
	{
		const FString& ClassA = Classes[i]->GetName();
		const FString SchemaTypeA = UnrealNameToSchemaTypeName(ClassA);

		for (int j = i + 1; j < Classes.Num(); ++j)
		{
			const FString& ClassB = Classes[j]->GetName();
			const FString SchemaTypeB = UnrealNameToSchemaTypeName(ClassB);

			if (SchemaTypeA.Equals(SchemaTypeB))
			{
				UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Class name collision after removing underscores: '%s' and '%s' - schema not generated"), *ClassA, *ClassB);
				return false;
			}
		}
	}

	// Ensure class conforms to schema uppercase letter check
	for (const auto& Class : Classes)
	{
		FString ClassName = Class->GetName();
		if (FChar::IsLower(ClassName[0]))
		{
			UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("SpatialType class begins with lowercase letter: %s. Schema not generated"), *ClassName);
			return false;
		}
	}
	return true;
}
}// ::

void GenerateSchemaFromClasses(const TArray<UClass*>& Classes, const FString& CombinedSchemaPath)
{
	Worker_ComponentId ComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;

	for (const auto& Class : Classes)
	{
		ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, ComponentId, Class);
	}
}

bool RunProcess(const FString& ExecutablePath, const FString& Arguments)
{
	TSharedPtr<FMonitoredProcess> Process = MakeShareable(new FMonitoredProcess(ExecutablePath, Arguments, true));
	Process->OnOutput().BindStatic(&OnStatusOutput);
	Process->Launch();
	// We currently spin on the thread calling this function as this appears to be
	// The idiomatic way according to the other usages of the FMonitoredProcess interface in the Unreal engine 
	// codebase. See TargetPlatformManagerModule.cpp for another example of this setup.
	while (Process->Update())
	{
		FPlatformProcess::Sleep(0.01f);
	}

	if (Process->GetReturnCode() != 0)
	{
		return false;
	}

	return true;
}

FString GenerateIntermediateDirectory()
{
	const FString CombinedIntermediatePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Intermediate/Improbable/"), *FGuid::NewGuid().ToString(), TEXT("/"));
	FString AbsoluteCombinedIntermediatePath = FPaths::ConvertRelativePathToFull(CombinedIntermediatePath);
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*AbsoluteCombinedIntermediatePath);

	return AbsoluteCombinedIntermediatePath;
}

void CreateSchemaDatabase(TArray<UClass*> Classes)
{
	AsyncTask(ENamedThreads::GameThread, [Classes]{
		FString PackagePath = TEXT("/Game/Spatial/SchemaDatabase");

		UPackage *Package = CreatePackage(nullptr, *PackagePath);

		USchemaDatabase* SchemaDatabase = NewObject<USchemaDatabase>(Package, USchemaDatabase::StaticClass(), FName("SchemaDatabase"), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);

		Worker_ComponentId ComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;

		for (UClass* Class : Classes)
		{
			FSchemaData SchemaData;
			SchemaData.SingleClientRepData = ComponentId++;
			SchemaData.MultiClientRepData = ComponentId++;
			SchemaData.HandoverData = ComponentId++;
			SchemaData.ClientRPCs = ComponentId++;
			SchemaData.ServerRPCs = ComponentId++;
			SchemaData.CrossServerRPCs = ComponentId++;
			SchemaData.NetMulticastRPCs = ComponentId++;

			SchemaDatabase->ClassToSchema.Add(Class, SchemaData);
		}

		FAssetRegistryModule::AssetCreated(SchemaDatabase);
		SchemaDatabase->MarkPackageDirty();

		FString FilePath = FString::Printf(TEXT("%s%s"), *PackagePath, *FPackageName::GetAssetPackageExtension());
		bool bSuccess = UPackage::SavePackage(Package, SchemaDatabase, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension()));

		if(!bSuccess)
		{
			FMessageDialog::Debugf(FText::FromString(FString::Printf(TEXT("Unable to save Schema Database to %s!"), *PackagePath)));
		}
	});
}

TArray<UClass*> GetAllSpatialTypeClasses()
{
	TArray<UClass*> Classes;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnySpatialClassFlags(SPATIALCLASS_GenerateTypeBindings) == false)
		{
			continue;
		}

		// Ensure we don't process skeleton or reinitialized classes
		if (It->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
			|| It->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		Classes.Add(*It);
	}

	return Classes;
}

TArray<UClass*> GetAllSupportedClasses()
{
	TArray<UClass*> Classes;

	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (*It == AActor::StaticClass()) continue;

		if (*It == UActorComponent::StaticClass()) continue;

		// Any component which has child components
		if (It->IsChildOf<USceneComponent>()) continue;

		if (It->GetName().Contains("AbilitySystemComponent")) continue;
		if (It->GetName().Contains("GameplayTasksComponent")) continue;

		if (!(It->IsChildOf<AActor>() || It->IsChildOf<UActorComponent>())) continue;

		// Doesn't let us save the schema database
		if (It->IsChildOf<ALevelScriptActor>()) continue;

		// Ensure we don't process skeleton or reinitialized classes
		if (It->GetName().StartsWith(TEXT("SKEL_"), ESearchCase::CaseSensitive)
			|| It->GetName().StartsWith(TEXT("REINST_"), ESearchCase::CaseSensitive))
		{
			continue;
		}

		Classes.Add(*It);
	}

	return Classes;
}

bool SpatialGDKGenerateInteropCode()
{
	const USpatialGDKEditorToolbarSettings* SpatialGDKToolbarSettings = GetDefault<USpatialGDKEditorToolbarSettings>();

	TArray<UClass*> InteropGeneratedClasses = GetAllSpatialTypeClasses();

	if (!CheckClassNameListValidity(InteropGeneratedClasses))
	{
		return false;
	}

	FString SchemaOutputPath = SpatialGDKToolbarSettings->GetGeneratedSchemaOutputFolder();

	UE_LOG(LogSpatialGDKInteropCodeGenerator, Display, TEXT("Schema path %s"), *SchemaOutputPath);

	// Check schema path is valid.
	if (!FPaths::CollapseRelativeDirectories(SchemaOutputPath))
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Invalid path: '%s'. Schema not generated."), *SchemaOutputPath);
		return false;
	}

	const FString SchemaIntermediatePath = GenerateIntermediateDirectory();
	GenerateSchemaFromClasses(InteropGeneratedClasses, SchemaIntermediatePath);

	const FString DiffCopyPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("Scripts/DiffCopy.bat")));
	// Copy schema files
	FString DiffCopyArguments = FString::Printf(TEXT("\"%s\" \"%s\" --verbose --remove-input"), *SchemaIntermediatePath, *SchemaOutputPath);
	if (!RunProcess(DiffCopyPath, DiffCopyArguments))
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error, TEXT("Could not move generated schema files during the diff-copy stage. Path: '%s', arguments: '%s'."), *DiffCopyPath, *DiffCopyArguments);
		return false;
	}

	CreateSchemaDatabase(InteropGeneratedClasses);

	return true;
}
