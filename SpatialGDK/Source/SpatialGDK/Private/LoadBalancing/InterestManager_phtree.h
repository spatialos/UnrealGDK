
THIRD_PARTY_INCLUDES_START
//// This is needed to fix memory alignment issues
//PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
#include "phtreelib.h"
//PRAGMA_POP_PLATFORM_DEFAULT_PACKING

THIRD_PARTY_INCLUDES_END

namespace SpatialGDK
{

float* AsFloatPtr(FVector2D const& Point)
{
	return reinterpret_cast<float*>(const_cast<FVector2D*>((&Point)));
}

struct FInterestManager::FBroadphaseImpl
{
	FBroadphaseImpl()
	{
		Tree = PH_Create2DFloatTree();
	}

	~FBroadphaseImpl()
	{
		PH_Delete2DFloatTree(Tree);
	}

	void Alloc()
	{
		uint32 Slot = Positions.Num();

		Positions.Push(FVector2D(INFINITY, INFINITY));
	}

	void SetViewer(uint32 , float)
	{}

	void Update(uint32 Slot, FVector Position)
	{
		PH_Remove2DFloat(Tree, AsFloatPtr(Positions[Slot]), Slot);
		Positions[Slot] = FVector2D(Position);
		PH_Insert2DFloat(Tree, AsFloatPtr(FVector2D(Position)), Slot);
	}

	void SwapEntries(int32 Slot1, int32 Slot2)
	{
		PH_Remove2DFloat(Tree, AsFloatPtr(Positions[Slot1]), Slot1);
		PH_Remove2DFloat(Tree, AsFloatPtr(Positions[Slot2]), Slot2);
		Swap(Positions[Slot1], Positions[Slot2]);
		PH_Insert2DFloat(Tree, AsFloatPtr(Positions[Slot1]), Slot1);
		PH_Insert2DFloat(Tree, AsFloatPtr(Positions[Slot2]), Slot2);
	}

	void MoveEntry(int32 SlotSrc, int32 SlotDest)
	{
		SwapEntries(SlotSrc, SlotDest);
	}

	void Remove()
	{
		PH_Remove2DFloat(Tree, AsFloatPtr(Positions.Last()), Positions.Num() - 1);
		Positions.Pop();
	}

	void ComputeBoxVisibility(const TArray<FBox2D>& Regions, uint64* OutVisibility) {}

	struct VisitCtx
	{
		VisitCtx(TArray<uint32>& Array)
			: OutArray(Array)
		{}

		static void OnLeaf(void* Self, uint32 Idx)
		{
			VisitCtx* Ctx = (VisitCtx*)Self;
			Ctx->OutArray.Add(Idx);
		}

		TArray<uint32>& OutArray;
	};

	struct VisitCtx_Mask
	{
		VisitCtx_Mask(uint32 InIdx, uint64* InOutMasks)
			: ViewerIdx(InIdx)
			, OutMasks(InOutMasks)
		{}

		static void OnLeaf(void* Self, uint32 Idx)
		{
			VisitCtx_Mask* Ctx = (VisitCtx_Mask*)Self;
			Ctx->OutMasks[Idx] |= 1ull<<Ctx->ViewerIdx;
		}

		uint32 ViewerIdx;
		uint64* OutMasks;
	};

	void ComputeVisibility(TArray<int32> Viewers, TArray<TArray<uint32>>& Out, float SearchRadius)
	{
		int32 const* ViewersPtr = Viewers.GetData();
		TArray<uint32>* ViewersArrayPtr = Out.GetData();
		for(int32 j = 0; j<Viewers.Num(); ++j)
		{
			uint32 CurViewer = *ViewersPtr;
			VisitCtx Ctx(ViewersArrayPtr[CurViewer]);
			PH_Circle2DVisitTree(Tree, AsFloatPtr(Positions[CurViewer]), SearchRadius, &VisitCtx::OnLeaf, &Ctx);

			++ViewersPtr;
		}
	}

	void ComputeVisibility_Mask(TArray<int32> Viewers, uint64* Out, float SearchRadius, float const* PosX, float const* PosY)
	{
		int32 const* ViewersPtr = Viewers.GetData();
		for (int32 j = 0; j < Viewers.Num(); ++j)
		{
			uint32 CurViewer = *ViewersPtr;
			VisitCtx_Mask Ctx(j, Out);
			PH_Circle2DVisitTree(Tree, AsFloatPtr(Positions[CurViewer]), SearchRadius, &VisitCtx_Mask::OnLeaf, &Ctx);

			++ViewersPtr;
		}
	}

	PhTree2DFloatWrapper* Tree;
	TArray<FVector2D> Positions;
};

} // namespace SpatialGDK
