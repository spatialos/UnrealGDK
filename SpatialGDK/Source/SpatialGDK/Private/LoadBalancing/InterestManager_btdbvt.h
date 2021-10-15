#define HAS_BULLET_PHYSICS

#ifdef HAS_BULLET_PHYSICS
// This is needed to suppress some warnings that UE4 escalates that Bullet doesn't
THIRD_PARTY_INCLUDES_START
// This is needed to fix memory alignment issues
PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
#include <BulletCollision/BroadphaseCollision/btDbvt.h>
PRAGMA_POP_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_END

namespace SpatialGDK
{
struct FInterestManager::FBroadphaseImpl
{
	FBroadphaseImpl() {}

	void Alloc()
	{
		uint32 Slot = Nodes.Num();
		btVector3 pos(0.0, 0.0, 0.0);
		auto box = btDbvtVolume::FromCE(pos, btVector3(0.1, 0.1, 0.1));

		Nodes.Push(nullptr);
		Positions.push_back(pos);
		Volumes.push_back(btDbvtVolume());
	}

	void SetViewer(uint32 , float){}

	void Update(uint32 Slot, FVector Position)
	{
		btVector3 pos(Position.X / 100, Position.Y / 100, Position.Z / 100);
		Positions[Slot] = pos;
		Volumes[Slot] = btDbvtVolume::FromCE(pos, btVector3(0.1, 0.1, 0.1));
		if(Nodes[Slot] != nullptr)
		{
			Tree.update(Nodes[Slot], Volumes[Slot], 1.0);
		}
		else
		{
			Nodes[Slot] = Tree.insert(Volumes[Slot], (void*)Slot);
		}
	}

	void SwapEntries(int32 Slot1, int32 Slot2)
	{
		Swap(Positions[Slot1], Positions[Slot2]);
		Swap(Volumes[Slot1], Volumes[Slot2]);
		Swap(Nodes[Slot1], Nodes[Slot2]);
	}

	void MoveEntry(int32 SlotSrc, int32 SlotDest)
	{
		SwapEntries(SlotSrc, SlotDest);
	}

	void Remove()
	{
		if(Nodes.Last() != nullptr)
		{
			Tree.remove(Nodes.Last());
		}
		Positions.pop_back();
		Volumes.pop_back();
		Nodes.Pop();
	}

	void ComputeBoxVisibility(const TArray<FBox2D>& Regions, uint64* OutVisibility) {}

	struct VisitStackElem
	{
		VisitStackElem(btDbvtNode* iNode, uint32 iListEnd)
			: node(iNode)
			, objListEnd(iListEnd)
		{
		}

		btDbvtNode* node;
		uint32 objListEnd;
		int32 curChild = -1;
	};

	void ComputeVisibility_Mask(TArray<int32> Viewers, uint64* Out, float SearchRadius, float const* PosX, float const* PosY)
	{
		Tree.optimizeIncremental(1);

		SearchRadius *= 0.01;

		const uint32 NumViewers = Viewers.Num();
		const uint32 NextMult4Viewers = ((NumViewers + 3) / 4) * 4;

		TArray<uint32> ViewerMaskSlot;
		ViewerMaskSlot.Reserve(NumViewers);
		for (uint32 i = 0; i < NumViewers; ++i)
		{
			ViewerMaskSlot.Add(i);
		}

		// Pad viewers up to 4.
		for(uint32 i = NumViewers; i<NextMult4Viewers; ++i)
		{
			Viewers.Add(0);
			ViewerMaskSlot.Add(0);
		}
		
		int32* ViewersPtr = Viewers.GetData();
		uint32* ViewerMaskSlotPtr = ViewerMaskSlot.GetData();

		TArray<VisitStackElem> nodes;
		nodes.Reserve(64);

		nodes.Add(VisitStackElem(Tree.m_root, NumViewers));

		btVector3* posArray = &Positions[0];
		btDbvtVolume* volumesArray = &Volumes[0];

		// btVector3 searchOffset = TO_BTVECT(iForwardOffset);
		// float inflateRadius = iRadiusSearch + searchOffset.length();
		float const sqRadius = SearchRadius * SearchRadius;
		;

		while (nodes.Num() > 0)
		{
			auto& curNode = nodes.Last();
			if (curNode.curChild == 1 || curNode.objListEnd <= 0)
			{
				nodes.Pop();
			}
			else
			{
				curNode.curChild++;

				btDbvtNode* child = curNode.node->childs[curNode.curChild];

				btDbvtVolume nodeExpandedVolume = child->volume;
				nodeExpandedVolume.Expand(btVector3(SearchRadius, SearchRadius, SearchRadius));

				if (!child->isleaf())
				{
					// Cull object list by children expanded volume
					nodes.Add(VisitStackElem(child, curNode.objListEnd));

					auto& stackElem = nodes.Last();
#if 1
					for (uint32 i = 0; i < stackElem.objListEnd; ++i)
					{
						btDbvtVolume const& objVolume = *(volumesArray + ViewersPtr[i]);
						if (!Intersect(nodeExpandedVolume, objVolume))
						{
							Swap(ViewersPtr[i], ViewersPtr[stackElem.objListEnd - 1]);
							Swap(ViewerMaskSlotPtr[i], ViewerMaskSlotPtr[stackElem.objListEnd - 1]);
							--stackElem.objListEnd;
							--i;
						}
					}
#else
					// Has bugs, do not use.
					int32* LocViewersPtr = ViewersPtr;
					uint32* LocViewerMaskSlotPtr = ViewerMaskSlotPtr;

					const uint32 NumElems4 = (stackElem.objListEnd + 3) / 4;
					const uint32 SaveEnd = stackElem.objListEnd;
					
					for (uint32 i = 0; i<NumElems4; ++i)
					{
						const uint32 ElemsToProcess = FMath::Min(((i + 1) * 4), SaveEnd) - i * 4;
						__m128i Slots;
						Slots = _mm_loadu_epi32(LocViewersPtr);
						VectorRegister RegX = _mm_i32gather_ps(PosX, Slots, sizeof(float));
						VectorRegister RegY = _mm_i32gather_ps(PosY, Slots, sizeof(float));
						VectorRegister Scaling = VectorSetFloat1(0.01);
						RegX = VectorMultiply(RegX, Scaling);
						RegY = VectorMultiply(RegY, Scaling);

						VectorRegister BoxTest = VectorSetFloat1(nodeExpandedVolume.Mins().x());
						VectorRegister TestResult = VectorCompareGT(RegX, BoxTest);

						BoxTest = VectorSetFloat1(nodeExpandedVolume.Maxs().x());
						TestResult = VectorBitwiseAnd(TestResult, VectorCompareGT(BoxTest, RegX));

						BoxTest = VectorSetFloat1(nodeExpandedVolume.Mins().y());
						TestResult = VectorBitwiseAnd(TestResult, VectorCompareGT(RegY, BoxTest));

						BoxTest = VectorSetFloat1(nodeExpandedVolume.Maxs().y());
						TestResult = VectorBitwiseAnd(TestResult, VectorCompareGT(BoxTest, RegY));

						uint32 Mask = VectorMaskBits(TestResult);

						int32* NextViewersPtr = LocViewersPtr + ElemsToProcess;
						uint32 NumValidElems = FMath::CountBits(Mask);
						if(NumValidElems >= 0 && NumValidElems < 4)
						{
							uint32 InvalidElems = ElemsToProcess - NumValidElems;

							int32* EndPtr = ViewersPtr + (stackElem.objListEnd - InvalidElems);
							uint32* EndMaskPtr = ViewerMaskSlotPtr + (stackElem.objListEnd - InvalidElems);

							int32 BufferBegin[2][4];
							int32 BufferEnd[2][4];

							uint32 ValidIdx = 0;
							uint32 InvalidIdx = 0;
							for(uint32 j = 0; j < ElemsToProcess ; ++j)
							{
								if((Mask & 1 << j) != 0)
								{
									BufferBegin[0][ValidIdx] = LocViewersPtr[j];
									BufferBegin[1][ValidIdx] = LocViewerMaskSlotPtr[j];
									++ValidIdx;
								}
								else
								{
									BufferEnd[0][InvalidIdx] = LocViewersPtr[j];
									BufferEnd[1][InvalidIdx] = LocViewerMaskSlotPtr[j];
									++InvalidIdx;
								}
							}
							uint32 k = 0;
							for (uint32 j = ValidIdx; j < ElemsToProcess; ++j, ++k)
							{
								BufferBegin[0][j] = EndPtr[k];
								BufferBegin[1][j] = EndMaskPtr[k];
							}

							memcpy(LocViewersPtr, BufferBegin[0], sizeof(uint32) * ElemsToProcess);
							memcpy(LocViewerMaskSlotPtr, BufferBegin[1], sizeof(uint32) * ElemsToProcess);

							memcpy(EndPtr, BufferEnd[0], sizeof(uint32) * InvalidIdx);
							memcpy(EndMaskPtr, BufferEnd[1], sizeof(uint32) * InvalidIdx);

							LocViewersPtr += NumValidElems;
							LocViewerMaskSlotPtr += NumValidElems;
							stackElem.objListEnd -= InvalidElems;
						}
						else if(NumValidElems == 4)
						{
							LocViewersPtr += 4;
							LocViewerMaskSlotPtr += 4;
						}
					}
#endif
				}
				else
				{
					union CastStruct
					{
						uint32 u32;
						void* ptr;
					} Slot;
					Slot.ptr = child->data;
					uint32 otherObj = Slot.u32;
#if 0
					for (uint32 i = 0; i < curNode.objListEnd; ++i)
					{
						uint32 curObject = ViewersPtr[i];
						if (curObject != otherObj && posArray[curObject].distance2(posArray[otherObj]) < sqRadius)
						{
							Out[otherObj] |= 1ull << ViewerMaskSlotPtr[i];
						}
					}
#else
					int32* LocViewersPtr = ViewersPtr;
					uint32* LocViewerMaskSlotPtr = ViewerMaskSlotPtr;

					VectorRegister Scaling = VectorSetFloat1(0.01);
					VectorRegister ObjPosX = VectorSetFloat1(PosX[otherObj]);
					VectorRegister ObjPosY = VectorSetFloat1(PosY[otherObj]);
					ObjPosX = VectorMultiply(ObjPosX, Scaling);
					ObjPosY = VectorMultiply(ObjPosY, Scaling);

					const uint32 NumElems4 = (curNode.objListEnd + 3) / 4;
					const uint32 SaveEnd = curNode.objListEnd;

					for (uint32 i = 0; i < NumElems4; ++i)
					{
						const uint32 ElemsToProcess = FMath::Min(((i + 1) * 4), SaveEnd) - i * 4;
						__m128i Slots;
						Slots = _mm_loadu_epi32(LocViewersPtr);
						VectorRegister RegX = _mm_i32gather_ps(PosX, Slots, sizeof(float));
						VectorRegister RegY = _mm_i32gather_ps(PosY, Slots, sizeof(float));
						
						RegX = VectorMultiply(RegX, Scaling);
						RegY = VectorMultiply(RegY, Scaling);

						VectorRegister DistX = VectorSubtract(ObjPosX, RegX);
						VectorRegister DistY = VectorSubtract(ObjPosY, RegY);
						DistX = VectorMultiply(DistX, DistX);
						DistY = VectorMultiply(DistY, DistY);
						VectorRegister SquaredDist = VectorAdd(DistX, DistY);
						VectorRegister TestResult = VectorCompareLT(SquaredDist, VectorSetFloat1(sqRadius));

						uint32 Mask = VectorMaskBits(TestResult);
						for(uint32 j = 0; j<ElemsToProcess; ++j)
						{
							if(Mask & 1<<j)
							{
								Out[otherObj] |= 1ull << LocViewerMaskSlotPtr[j];
							}
						}
						LocViewersPtr += 4;
						LocViewerMaskSlotPtr += 4;
					}
#endif
				}
			}
		}
	}

	void ComputeVisibility(TArray<int32> Viewers, TArray<TArray<uint32>>& Out, float SearchRadius)
	{
		SearchRadius *= 0.01;

		const uint32 NumViewers = Viewers.Num();
		int32* ViewersPtr = Viewers.GetData();

		TArray<VisitStackElem> nodes;
		nodes.Reserve(64);

		nodes.Add(VisitStackElem(Tree.m_root, NumViewers));

		btVector3* posArray = &Positions[0];
		btDbvtVolume* volumesArray = &Volumes[0];

		// btVector3 searchOffset = TO_BTVECT(iForwardOffset);
		// float inflateRadius = iRadiusSearch + searchOffset.length();
		float const sqRadius = SearchRadius * SearchRadius;
		;

		while (nodes.Num() > 0)
		{
			auto& curNode = nodes.Last();
			if (curNode.curChild == 1 || curNode.objListEnd <= 0)
			{
				nodes.Pop();
			}
			else
			{
				curNode.curChild++;
				btDbvtNode* child = curNode.node->childs[curNode.curChild];

				btDbvtVolume nodeExpandedVolume = child->volume;
				nodeExpandedVolume.Expand(btVector3(SearchRadius, SearchRadius, SearchRadius));

				if (!child->isleaf() && curNode.objListEnd > 0)
				{
					// Cull object list by children expanded volume
					nodes.Add(VisitStackElem(child, curNode.objListEnd));

					auto& stackElem = nodes.Last();

					for (int i = 0; i < (int)stackElem.objListEnd; ++i)
					{
						btDbvtVolume const& objVolume = *(volumesArray + ViewersPtr[i]);
						if (!Intersect(nodeExpandedVolume, objVolume))
						{
							Swap(ViewersPtr[i], ViewersPtr[stackElem.objListEnd - 1]);
							--stackElem.objListEnd;
							--i;
						}
					}
				}
				else
				{
					union CastStruct
					{
						uint32 u32;
						void* ptr;
					} Slot;
					Slot.ptr = child->data;
					uint32 otherObj = Slot.u32;
					for (uint32 i = 0; i < curNode.objListEnd; ++i)
					{
						uint32 curObject = ViewersPtr[i];
						if (curObject != otherObj && posArray[curObject].distance2(posArray[otherObj]) < sqRadius)
						{
							Out[curObject].Add(otherObj);
						}
					}
				}
			}
		}
	}

	btDbvt Tree;
	TArray<btDbvtNode*> Nodes;
	btAlignedObjectArray<btVector3> Positions;
	btAlignedObjectArray<btDbvtVolume> Volumes;
};

} // namespace SpatialGDK

#endif
