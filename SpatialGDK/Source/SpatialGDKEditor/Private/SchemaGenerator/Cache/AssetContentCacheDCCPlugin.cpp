

#include "AssetContentCacheDCCPlugin.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/SecureHash.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "UObject/LinkerLoad.h"
#include "AssetRegistryModule.h"
#include "SchemaGenerator/Cache/SchemaDCCPlugin.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

FMD5Hash FAssetContentSummary::ComputeHash()
{
	if (AssetDependencies.Num() == 0)
	{
		return AssetFileHash;
	}

	FMD5 Hash;
	Hash.Update(AssetFileHash.GetBytes(), AssetFileHash.GetSize());
	for (const auto& Dep : AssetDependencies)
	{
		Hash.Update(Dep.GetBytes(), Dep.GetSize());
	}

	FMD5Hash Result;
	Result.Set(Hash);
	return Result;
}

struct DepNode
{
	TSet<FName> Dependencies;
	TSet<FName> DependenciesToSatisfy;
	TSet<FName> Dependents;
	uint32 NodeId;
	int32 Level = -1;
};

void GenerateViz(TMap<FName, DepNode*>& Packages)
{
	FString OutGraph;
	OutGraph.Append("graph G {\n");
	for (auto const& Entry : Packages)
	{
		FString NodeName = FString::Printf(TEXT("Node%i"), Entry.Value->NodeId);
		OutGraph.Append(FString::Printf(TEXT("%s [label = \"%s\" ]\n"), *NodeName, *Entry.Key.ToString()));
	}
	for (auto const& Entry : Packages)
	{
		FString NodeName = FString::Printf(TEXT("Node%i"), Entry.Value->NodeId);
		for (auto const& Dep : Entry.Value->Dependencies)
		{
			DepNode& Node = *Packages[Dep];
			FString DepName = FString::Printf(TEXT("Node%i"), Node.NodeId);
			OutGraph.Append(FString::Printf(TEXT("%s -> %s\n"), *NodeName, *DepName));
		}
	}

	OutGraph.Append("}\n");

	if (FArchive* OutFile = IFileManager::Get().CreateDebugFileWriter(TEXT("C:\\Dev\\DebugGraph.txt")))
	{
		*OutFile << OutGraph;

		delete OutFile;
	}
}

FMD5Hash GetFileHash(const FName& PackageName)
{
	FString FileName1 = FPackageName::LongPackageNameToFilename(PackageName.ToString());

	FString FileName2;
	FPackageName::FindPackageFileWithoutExtension(FileName1, FileName2);


	if(IFileManager::Get().FileExists(*FileName2))
	{
		return FMD5Hash::HashFile(*FileName2);
	}
	else
	{
		FString PackageNameString = PackageName.ToString();
		check(!PackageNameString.StartsWith("/Game"));

		FMD5 Hash;
		Hash.Update(reinterpret_cast<uint8*>(PackageNameString.GetCharArray().GetData()), PackageNameString.GetCharArray().Num() * sizeof(TCHAR));

		FMD5Hash Result;
		Result.Set(Hash);

		return Result;
	}
}

TOptional<FMD5Hash> FAssetContentIdentifierCache::GetAssetIdentifier(FName PackageName)
{
	if (const auto* Hash = AssetSignatures.Find(PackageName))
	{
		return *Hash;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FName> Dependencies;

	TMap<FName, DepNode*> VisitedPackages;
	TArray<FName> PackagesQueue;

	PackagesQueue.Add(PackageName);

	TSet<FName> RootPackages;
	while (PackagesQueue.Num() != 0)
	{
		Dependencies.Empty();
		FName CurPackage = PackagesQueue.Pop();
		DepNode* CurNode = (VisitedPackages.Find(CurPackage) ? *VisitedPackages.Find(CurPackage) : nullptr);
		if (CurNode == nullptr)
		{
			CurNode = new DepNode;
			CurNode->NodeId = VisitedPackages.Num();
			VisitedPackages.Add(CurPackage, CurNode);
		}

		if (AssetSignatures.Find(CurPackage))
		{
			CurNode->Level = 0;
			RootPackages.Add(CurPackage);
			continue;
		}

		AssetRegistryModule.Get().GetDependencies(CurPackage, Dependencies, EAssetRegistryDependencyType::Hard);

		for (const FName& DepPackage : Dependencies)
		{
			if (DepPackage != PackageName)
			{
				bool bNewNode = false;
				DepNode* Node = (VisitedPackages.Find(DepPackage) ? *VisitedPackages.Find(DepPackage) : nullptr);
				if (Node == nullptr)
				{
					Node = new DepNode;
					Node->NodeId = VisitedPackages.Num() - 1;
					VisitedPackages.Add(DepPackage, Node);
					bNewNode = true;
				}

				Node->Dependents.Add(CurPackage);
				CurNode->Dependencies.Add(DepPackage);

				if (bNewNode)
				{
					PackagesQueue.Add(DepPackage);
				}

				auto Res = AssetSignatures.Find(DepPackage);
				if (Res == nullptr)
				{
					CurNode->DependenciesToSatisfy.Add(DepPackage);
				}
				else
				{
					if (!(*Res))
					{
						return TOptional<FMD5Hash>();
					}
				}
			}
		}

		if(CurNode->Dependencies.Num() == 0)
		{
			CurNode->Level = 0;
			RootPackages.Add(CurPackage);
		}
	}

	GenerateViz(VisitedPackages);

	TSet<FName> Dependents;

	if (RootPackages.Num() == 0)
	{
		UE_LOG(LogUObjectGlobals, Warning, TEXT("Circular dependency for asset %s "), *PackageName.ToString());
		AssetSignatures.Add(PackageName, TOptional<FMD5Hash>());
		return TOptional<FMD5Hash>();
	}

	for (auto& Package : RootPackages)
	{
		const DepNode& PackageNode = *VisitedPackages[Package];
		if (AssetSignatures.Find(Package) == nullptr)
		{
			check(PackageNode.Dependencies.Num() == 0);
			AssetSignatures.Add(Package, GetFileHash(PackageName));
		}
	}

	TSet<FName> NextRoot;
	if (VisitedPackages.Num() > 1)
	{
		bool bReachedTop = RootPackages.Num() == 1 && RootPackages.Contains(PackageName);
		while (!bReachedTop)
		{
			bReachedTop = RootPackages.Num() == 1 && RootPackages.Contains(PackageName);
			for (auto& Package : RootPackages)
			{
				const DepNode& PackageNode = *VisitedPackages[Package];
				for (auto& Dependent : PackageNode.Dependents)
				{
					DepNode& DependentNode = *VisitedPackages[Dependent];
					//if (DependentNode.Level != -1 && DependentNode.Level < PackageNode.Level)
					//{
					//	UE_LOG(LogUObjectGlobals, Warning, TEXT("Circular dependency between %s and %s"), *Package.ToString(), *Dependent.ToString());
					//	AssetSignatures.Add(Dependent, TOptional<FMD5Hash>());
					//	AssetSignatures.Add(Package, TOptional<FMD5Hash>());
					//	return TOptional<FMD5Hash>();
					//}
					DependentNode.Level = PackageNode.Level + 1;
					DependentNode.DependenciesToSatisfy.Remove(Package);
					if (DependentNode.DependenciesToSatisfy.Num() == 0)
					{
						NextRoot.Add(Dependent);
					}
				}
			}

			if (NextRoot.Num() == 0)
			{
				if (!bReachedTop)
				{
					UE_LOG(LogUObjectGlobals, Warning, TEXT("Circular dependency for asset %s "), *PackageName.ToString());
					AssetSignatures.Add(PackageName, TOptional<FMD5Hash>());
					return TOptional<FMD5Hash>();
				}
			}

			for (auto const& Package : NextRoot)
			{
				FAssetContentSummary ContentHash;

				ContentHash.AssetFileHash = GetFileHash(Package);

				const DepNode& PackageNode = *VisitedPackages[Package];
				for (auto& Dep : PackageNode.Dependencies)
				{
					if (auto Hash = AssetSignatures.FindOrAdd(Dep))
					{
						ContentHash.AssetDependencies.Add(Hash.GetValue());
					}
					else
					{
						return TOptional<FMD5Hash>();
					}
				}

				ContentHash.AssetDependencies.Sort([](const FMD5Hash& Hash1, const FMD5Hash& Hash2)
				{
					if (Hash1.GetSize() != Hash2.GetSize())
					{
						return Hash1.GetSize() < Hash2.GetSize();
					}
					return FMemory::Memcmp(Hash1.GetBytes(), Hash2.GetBytes(), Hash1.GetSize()) < 0;
				});

				FMD5Hash Hash = ContentHash.ComputeHash();

				AssetSignatures.Add(Package, Hash);
			}

			RootPackages = MoveTemp(NextRoot);
		}
	}

	if (auto Hash = AssetSignatures.FindOrAdd(PackageName))
	{
		return Hash;
	}
	else
	{
		return TOptional<FMD5Hash>();
	}
}

FAssetSpatialContentCache::FAssetSpatialContentCache(const FName& Asset, FAssetContentIdentifierCache& InIDCache)
	: AssetToInspect(Asset)
	, IDCache(InIDCache)
{
	
}

const TCHAR* FAssetSpatialContentCache::GetPluginName() const
{
	return TEXT("SpatialAssetContentCache");
}

const TCHAR* FAssetSpatialContentCache::GetVersionString() const
{
	return TEXT("B36DA8F3-1B3F-48CE-B088-4C14BFD5251B");
}

FString FAssetSpatialContentCache::GetPluginSpecificCacheKeySuffix() const
{
	if (auto Hash = IDCache.GetAssetIdentifier(AssetToInspect))
	{
		return LexToString(Hash.GetValue());
	}
	return "Empty";
}

bool FAssetSpatialContentCache::IsBuildThreadsafe() const
{
	return false;
}

bool FAssetSpatialContentCache::IsDeterministic() const
{
	return true;
}

FString FAssetSpatialContentCache::GetDebugContextString() const
{
	return FString::Printf(TEXT("Spatial asset content cache : %s"), *AssetToInspect.ToString());
}

namespace SpatialGDKEditor
{
	namespace Schema
	{
		bool IsSupportedClass(const UClass* SupportedClass);
	}
}

bool FAssetSpatialContentCache::Build(TArray<uint8>& OutData)
{
	if (auto Hash = IDCache.GetAssetIdentifier(AssetToInspect))
	{

	}
	else
	{
		return false;
	}

	UPackage* Package = LoadPackage(nullptr, *AssetToInspect.ToString(), LOAD_NoRedirects);

	if (!Package)
	{
		return false;
	}

	FSpatialAssetContentSummary SpatialContent;

	if (!Package->LinkerLoad)
	{
		return true;
	}
	for (auto& Entry : Package->LinkerLoad->ExportMap)
	{
		UClass* ExportedClass = Cast<UClass>(Entry.Object);
		if (ExportedClass && SpatialGDKEditor::Schema::IsSupportedClass(ExportedClass))
		{
			FSchemaClassCache IDProvider(ExportedClass);
			SpatialContent.SpatialClasses.Add(IDProvider.GetPluginSpecificCacheKeySuffix());

			if(ExportedClass->IsChildOf<AActor>())
			{
				SpatialContent.NetCullDistances.Add(ExportedClass->GetDefaultObject<AActor>()->NetCullDistanceSquared);
			}
		}
	}

	FMemoryWriter Writer(OutData);
	Writer << SpatialContent.SpatialClasses;
	Writer << SpatialContent.NetCullDistances;

	return true;
}

FSpatialAssetContentSummary FAssetSpatialContentCache::ReadCachedData(const TArray<uint8>& CachedData)
{
	FSpatialAssetContentSummary Result;
	FMemoryReader Reader(CachedData);

	Reader << Result.SpatialClasses;
	Reader << Result.NetCullDistances;

	return Result;
}
