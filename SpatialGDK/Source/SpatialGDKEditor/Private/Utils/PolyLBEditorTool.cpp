// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/PolyLBEditorTool.h"

#include "LoadBalancing/LBCellVolume.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "KismetProceduralMeshLibrary.h"
#include "LevelEditorViewport.h"
#include "ProceduralMeshComponent.h"
#include "Templates/Sorting.h"
#include "Voronoi/Voronoi.h"

#pragma optimize("", off)

ALBPolyPreview::ALBPolyPreview()
{
	PrimaryActorTick.bCanEverTick = false;

	bIsEditorOnlyActor = true;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	Mesh->SetSimulatePhysics(false);
}

UPolyLBEditorTool::UPolyLBEditorTool()
{
	WorldExtents = FBox::BuildAABB(FVector(0, 0, 0), FVector(50000, 50000, 50000));
}

void UPolyLBEditorTool::Preview()
{
	UWorld* World = GCurrentLevelEditingViewportClient->GetWorld();
	ULevel* CurLevel = World->GetCurrentLevel();

	TArray<FRadicalVoronoiSite> VoroSites;

	for (TActorIterator<ALBPolyPreview> Iter(World, ALBPolyPreview::StaticClass()); Iter; ++Iter)
	{
		(*Iter)->Destroy();
	}

	// for (AActor* Actor : CurLevel->Actors)
	//{
	//	if(Cast<ALBPolyPreview>(Actor))
	//	{
	//		GEditor->AddActor
	//}

	for (TActorIterator<ALBCellVolume> Iter(World, ALBCellVolume::StaticClass()); Iter; ++Iter)
	{
		FRadicalVoronoiSite NewSite;
		ALBCellVolume* Volume = *Iter;
		NewSite.Position = Volume->GetActorLocation();
		NewSite.Radius = Volume->GetBounds().SphereRadius;

		VoroSites.Add(NewSite);
	}

	FVoronoiDiagram Diagram(VoroSites, WorldExtents, 1.0);

	TArray<FVoronoiCellInfo> Cells;
	Diagram.ComputeAllCells(Cells);

	for (auto const& Cell : Cells)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector2D CellCenter(EForceInit::ForceInitToZero);
		FBox CellBox(Cell.Vertices);
		TSet<FVector2D> Vtx2D;
		for (auto Vtx : Cell.Vertices)
		{
			Vtx2D.Add(FVector2D(Vtx));
		}

		for (auto Vtx : Vtx2D)
		{
			CellCenter += Vtx;
		}
		CellCenter /= Vtx2D.Num();

		FTransform ActorTrans(FVector(CellCenter, 0.0));

		ALBPolyPreview* PreviewActor = World->SpawnActor<ALBPolyPreview>(ALBPolyPreview::StaticClass(), ActorTrans, SpawnInfo);
		// ALBPolyPreview* PreviewActor = Cast<ALBPolyPreview>(GEditor->AddActor(CurLevel, ALBPolyPreview::StaticClass(), ActorTrans));
		UProceduralMeshComponent* Mesh = PreviewActor->Mesh;

		TArray<FVector> Vertices;
		for (auto Vtx : Vtx2D)
		{
			Vertices.Add(FVector(Vtx - CellCenter, 0.0));
		}
		Sort(Vertices.GetData() + 1, Vertices.Num() - 1, [OrigPt = Vertices[0]](FVector const& iVtx1, FVector const& iVtx2) {
			return FVector::CrossProduct((iVtx1 - OrigPt), (iVtx2 - OrigPt)).Z > 0;
		});

		int32 NumCellVtx = Vertices.Num();
		Vertices.Add(FVector(CellCenter, 0));
		int32 NumVtx2D = Vertices.Num();

		for (int32 i = 0; i < NumVtx2D; ++i)
		{
			Vertices.Add(FVector(FVector2D(Vertices[i]), 100));
		}

		TArray<FVector2D> UV0;
		for (auto const& Vtx : Vertices)
		{
			FVector RelPos = (Vtx - CellBox.Min) / CellBox.GetSize();
			UV0.Add(FVector2D(RelPos));
		}

		TArray<FVector> Normals;
		for (auto const& Vtx : Vertices)
		{
			Normals.Add(FVector(0, 0, 1));
		}

		TArray<FColor> VertexColors;
		VertexColors.Init(FColor(1.0, 1.0, 1.0, 1.0), Vertices.Num());

		TArray<int32> Triangles;
		for (int32 i = 0; i < NumCellVtx; ++i)
		{
			int32 NextIdx = (i + 1) % NumCellVtx;

			Triangles.Add(NumCellVtx);
			Triangles.Add(i);
			Triangles.Add(NextIdx);

			Triangles.Add(i);
			Triangles.Add(NextIdx);
			Triangles.Add(i + NumVtx2D);

			Triangles.Add(i);
			Triangles.Add(NextIdx + NumVtx2D);
			Triangles.Add(NextIdx);

			Triangles.Add(NumCellVtx + NumVtx2D);
			Triangles.Add(i + NumVtx2D);
			Triangles.Add(NextIdx + NumVtx2D);
		}

		TArray<FProcMeshTangent> Tangents;
		UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);

		Mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, false);
	}
}

void UPolyLBEditorTool::Save() {}
