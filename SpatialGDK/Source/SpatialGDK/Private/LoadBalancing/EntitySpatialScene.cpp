#include "LoadBalancing/EntitySpatialScene.h"

// This is needed to suppress some warnings that UE4 escalates that Bullet doesn't
THIRD_PARTY_INCLUDES_START
// This is needed to fix memory alignment issues
PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
#include <BulletCollision/BroadphaseCollision/btDbvt.h>
PRAGMA_POP_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_END

#include <algorithm>
#include <cmath>

namespace SpatialGDK
{
struct FEntitySpatialScene::Impl
{
	Impl() {}

	void Add(Worker_EntityId EntityId, FVector Position)
	{
		btVector3 pos(Position.X / 100, Position.Y / 100, Position.Z / 100);
		auto box = btDbvtVolume::FromCE(pos, btVector3(1, 1, 1));

		btDbvtNode* node = Tree.insert(box, (void*)EntityId);
		node->data = (void*)EntityId;

		int32 TreeId = Nodes.Num();

		Nodes.Push(node);
		Transforms.push_back(btTransform::getIdentity());
		Transforms[TreeId].setOrigin(pos);
		Volumes.expand(btDbvtVolume::FromCE(pos, btVector3(1, 1, 1)));

		EntityToTreeSlot.Add(EntityId, TreeId);
	}

	void Update(Worker_EntityId EntityId, FVector Position)
	{
		if (auto TreeId = EntityToTreeSlot.Find(EntityId))
		{
			btVector3 pos(Position.X / 100, Position.Y / 100, Position.Z / 100);
			Transforms[*TreeId].setOrigin(pos);
			Volumes[*TreeId] = btDbvtVolume::FromCE(pos, btVector3(1, 1, 1));
			Tree.update(Nodes[*TreeId], Volumes[*TreeId], 1.0);
			Dirty = true;
		}
	}

	void Remove(Worker_EntityId EntityId)
	{
		if (auto TreeId = EntityToTreeSlot.Find(EntityId))
		{
			if (Nodes.Num() > 0)
			{
				Worker_EntityId ToRelocate = (Worker_EntityId)Nodes.Last()->data;
				int32* LastSlot = EntityToTreeSlot.Find(ToRelocate);

				Transforms[*TreeId] = Transforms[*LastSlot];
				Volumes[*TreeId] = Volumes[*LastSlot];
				Nodes[*TreeId] = Nodes[*LastSlot];
				*LastSlot = *TreeId;
			}

			Tree.remove(Nodes[*TreeId]);
			Transforms.pop_back();
			Volumes.pop_back();
			Nodes.Pop();
			EntityToTreeSlot.Remove(EntityId);
		}
	}

	btDbvt Tree;
	TMap<Worker_EntityId_Key, int32> EntityToTreeSlot;

	TArray<btDbvtNode*> Nodes;
	btAlignedObjectArray<btTransform> Transforms;
	btAlignedObjectArray<btDbvtVolume> Volumes;

	bool Dirty = false;
};

FEntitySpatialScene::~FEntitySpatialScene() = default;

FEntitySpatialScene::FEntitySpatialScene()
	: m_Impl(MakeUnique<Impl>())
{
	NeighboursInfo.Reserve(1 << 14);
}

void FEntitySpatialScene::Add(Worker_EntityId EntityId, FVector Position, bool bUsesNN)
{
	m_Impl->Add(EntityId, Position);

	if (bUsesNN)
	{
		int32 NewId = NeighboursInfo.Num();
		NeighboursInfo.Add(FNeighborEntry());
		NeighboursInfo[NewId].EntityId = EntityId;

		EntityToSlot.Add(EntityId, NewId);
	}
}

void FEntitySpatialScene::Update(Worker_EntityId EntityId, FVector Position)
{
	m_Impl->Update(EntityId, Position);
}

void FEntitySpatialScene::Remove(Worker_EntityId EntityId)
{
	m_Impl->Remove(EntityId);
	if (int32* Slot = EntityToSlot.Find(EntityId))
	{
		if (NeighboursInfo.Num() > 0)
		{
			Worker_EntityId ToRelocate = NeighboursInfo.Last().EntityId;
			int32* LastSlot = EntityToSlot.Find(ToRelocate);

			NeighboursInfo[*Slot] = NeighboursInfo.Last();
			*LastSlot = *Slot;
		}

		NeighboursInfo.Pop();

		EntityToSlot.Remove(EntityId);
	}
}

struct ObjSort : btDbvt::ICollide
{
	ObjSort(TMap<Worker_EntityId, int32> const& iNeighMap, TMap<Worker_EntityId, int32> const& iTreeMap,
			TArray<FNeighborEntry> const& iDescs, TArray<int32>& oIds, TArray<int32>& oNeighIds)
		: m_NeighMap(iNeighMap)
		, m_TreeMap(iTreeMap)
		, m_OutIds(oIds)
		, m_OutNeighIds(oNeighIds)
		, m_Descs(iDescs)
	{
	}
	TMap<Worker_EntityId, int32> const& m_NeighMap;
	TMap<Worker_EntityId, int32> const& m_TreeMap;
	TArray<FNeighborEntry> const& m_Descs;
	TArray<int32>& m_OutIds;
	TArray<int32>& m_OutNeighIds;
	void Process(const btDbvtNode* leaf)
	{
		auto iter = m_NeighMap.Find((Worker_EntityId)leaf->data);
		if (iter != nullptr && /*m_Descs[*iter].populateNeigh*/ true)
		{
			m_OutIds.Add(*m_TreeMap.Find((Worker_EntityId)leaf->data));
			m_OutNeighIds.Add(*iter);
		}
	}
};

struct VisitStackElem
{
	VisitStackElem(btDbvtNode* iNode, uint32_t iListEnd)
		: node(iNode)
		, objListEnd(iListEnd)
	{
	}

	btDbvtNode* node;
	uint32_t objListEnd;
	int32_t curChild = -1;
};

bool FEntitySpatialScene::UpdateNeighbours(float SearchRadius)
{
	if (!m_Impl->Dirty)
	{
		return false;
	}

	m_Impl->Tree.optimizeIncremental(1);
	m_Impl->Dirty = false;

	TArray<int32> objects;
	TArray<int32> neighSlot;
	objects.Reserve(NeighboursInfo.Num());
	{
		ObjSort sort(EntityToSlot, m_Impl->EntityToTreeSlot, NeighboursInfo, objects, neighSlot);
		btDbvt::enumLeaves(m_Impl->Tree.m_root, sort);
	}

	if (objects.Num() == 0)
	{
		return true;
	}

	for (auto& neigh : NeighboursInfo)
	{
		neigh.NumNeighbours = 0;
	}

	TArray<VisitStackElem> nodes;
	nodes.Reserve(64);

	nodes.Add(VisitStackElem(m_Impl->Tree.m_root, neighSlot.Num()));

	FNeighborEntry* neighborsArray = NeighboursInfo.GetData();
	btTransform* transArray = &m_Impl->Transforms[0];
	btDbvtVolume* volumesArray = &m_Impl->Volumes[0];

	// btVector3 searchOffset = TO_BTVECT(iForwardOffset);
	// float inflateRadius = iRadiusSearch + searchOffset.length();
	SearchRadius *= 0.01;
	float const sqRadius = SearchRadius * SearchRadius;
	constexpr int32 s_NumNeigh = FNeighborEntry::s_MaxNumNeighbours;

	while (nodes.Num() > 0)
	{
		auto& curNode = nodes.Last();
		if (curNode.curChild == 1)
		{
			nodes.Pop();
		}
		else
		{
			curNode.curChild++;
			btDbvtNode* child = curNode.node->childs[curNode.curChild];

			btDbvtVolume nodeExpandedVolume = child->volume;
			nodeExpandedVolume.Expand(btVector3(SearchRadius, SearchRadius, SearchRadius));

			if (child->isleaf())
			{
				Worker_EntityId otherObjId = (Worker_EntityId)child->data;
				auto iter = m_Impl->EntityToTreeSlot.Find(otherObjId);
				if (iter /*!= m_UsrDataToNum.end()*/)
				{
					int32_t otherObj = *iter;
					btVector3 const& otherPos = (transArray + otherObj)->getOrigin();

					// Update neighborhood with remaining object.
					for (uint32_t i = 0; i < curNode.objListEnd; ++i)
					{
						uint32_t curObject = objects[i];
						FNeighborEntry& neighToUpdate = neighborsArray[neighSlot[i]];

						if (curObject != otherObj)
						{
							btDbvtVolume const& objVolume = volumesArray[curObject];

							if (Intersect(nodeExpandedVolume, objVolume))
							{
								// Neigh& otherNeigh = *(neighborsArray + otherObj);
								btTransform const& curObjTrans = transArray[curObject];
								// btVector3 curObjPos = *(transArray + curObject);

								btVector3 const& curObjPos = curObjTrans.getOrigin();
								btVector3 const& curObjFrontDir = curObjTrans.getBasis().getColumn(0);

								float sqLen;
								// float conePos;
								// if (neighToUpdate.m_Shape == Neigh::Sphere)
								{
									// if (otherNeigh.m_Shape == Neigh::Sphere)
									{
										sqLen = curObjPos.distance2(otherPos);
										// conePos = (otherPos - curObjPos).normalized().dot(curObjFrontDir);
									}
									// else if (otherNeigh.m_Shape == Neigh::Box)
									//{
									//	//Pretend we are on a plane :/
									//	AABB2Df box2D;
									//	box2D.m_Data[0] = MathTools::As2DVec(FROM_BTVECT(child->volume.Mins()));
									//	box2D.m_Data[1] = MathTools::As2DVec(FROM_BTVECT(child->volume.Maxs()));
									//
									//	Vector2f ptDists = box2D.Classify(MathTools::As2DVec(FROM_BTVECT(curObjPos)));
									//
									//	sqLen = ptDists.SquaredLength();
									//}
								}

								float* vecBegin = neighToUpdate.SquaredDist;
								float* vecEnd = neighToUpdate.SquaredDist + neighToUpdate.NumNeighbours;
								float* vecInsert = std::upper_bound(vecBegin, vecEnd, sqLen);

								if (vecInsert != vecEnd)
								{
									int32 DestSlot = (vecInsert - vecBegin);
									Worker_EntityId* neighInsert = neighToUpdate.Neighbours + DestSlot;
									uint32_t sizeCopy = FMath::Min(neighToUpdate.NumNeighbours, s_NumNeigh - 1) - DestSlot;
									memmove(vecInsert + 1, vecInsert, sizeCopy * sizeof(float));
									memmove(neighInsert + 1, neighInsert, sizeCopy * sizeof(Worker_EntityId));

									*neighInsert = otherObjId;
									*vecInsert = sqLen;
									if (neighToUpdate.NumNeighbours < s_NumNeigh)
									{
										++neighToUpdate.NumNeighbours;
									}
								}
								else if (neighToUpdate.NumNeighbours < s_NumNeigh)
								{
									neighToUpdate.Neighbours[neighToUpdate.NumNeighbours] = otherObjId;
									neighToUpdate.SquaredDist[neighToUpdate.NumNeighbours] = sqLen;
									++neighToUpdate.NumNeighbours;
								}
							}
						}
					}
				}
			}
			else
			{
				// Cull object list by children expanded volume
				nodes.Add(VisitStackElem(child, curNode.objListEnd));

				auto& stackElem = nodes.Last();

				for (int i = 0; i < (int)stackElem.objListEnd; ++i)
				{
					btDbvtVolume const& objVolume = *(volumesArray + objects[i]);
					if (!Intersect(nodeExpandedVolume, objVolume))
					{
						std::swap(objects[i], objects[stackElem.objListEnd - 1]);
						std::swap(neighSlot[i], neighSlot[stackElem.objListEnd - 1]);
						--stackElem.objListEnd;
						--i;
					}
				}
			}
		}
	}

	return true;
}
} // namespace SpatialGDK
