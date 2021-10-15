#define HAS_BULLET_PHYSICS

#include "CoreMinimal.h"

#ifdef HAS_BULLET_PHYSICS
// This is needed to suppress some warnings that UE4 escalates that Bullet doesn't
THIRD_PARTY_INCLUDES_START
// This is needed to fix memory alignment issues
PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
#include <btBulletCollisionCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include "btHashed32OverlappingPairCache.h"
PRAGMA_POP_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_END

namespace SpatialGDK
{
	class btCustomDispatcher : public btCollisionDispatcher
	{
	public:
		btCustomDispatcher(btCollisionConfiguration* config)
			: btCollisionDispatcher(config)
		{

		}

		bool needsCollision(const btCollisionObject* body0, const btCollisionObject* body1) override
		{
			if (body0->getUserIndex() == body1->getUserIndex())
			{
				return false;
			}

			//if (body0->getUserIndex2() == body1->getUserIndex2())
			//{
			//	return false;
			//}
			
			return true;
		}

		virtual bool needsResponse(const btCollisionObject* body0, const btCollisionObject* body1) override
		{
			return false;
		}
	};

	class ViewerObject : public btGhostObject
	{
	public:
		void addOverlappingObjectInternal(btBroadphaseProxy* otherProxy, btBroadphaseProxy* thisProxy) override
		{
			btCollisionObject* otherObject = (btCollisionObject*)otherProxy->m_clientObject;

			uint32 Slot = otherObject->getUserIndex();
			if (Slot == getUserIndex())
			{
				return;
			}
			Added.Add(Slot);
		}

		void removeOverlappingObjectInternal(btBroadphaseProxy* otherProxy, btDispatcher* dispatcher, btBroadphaseProxy* thisProxy) override
		{
			btCollisionObject* otherObject = (btCollisionObject*)otherProxy->m_clientObject;

			uint32 Slot = otherObject->getUserIndex();
			if (Slot == getUserIndex())
			{
				return;
			}
			Removed.Add(Slot);
		}

		TArray<uint32> Added;
		TArray<uint32> Removed;
	};

	struct FInterestManager::FBroadphaseImpl
	{
		FBroadphaseImpl()
			: Dispatcher(&CollisionConfig)
			, Broadphase(&PairCache)
			, World(&Dispatcher, &Broadphase, &CollisionConfig)
			, DefaultShape(0.1)
		{
			CollisionConfig.setPlaneConvexMultipointIterations();
			Broadphase.m_deferedcollide = true;
			static_cast<btOverlappingPairCache&>(PairCache).setInternalGhostPairCallback(&GhostCB);
		}

		void Alloc()
		{
			uint32 Slot = Nodes.Num();
			Nodes.Push(nullptr);
		}

		void SetViewer(uint32 Slot, float ViewerRadius)
		{
			ViewerObject* Viewer = Viewers.Add(Slot, new ViewerObject);
			Viewer->setUserIndex(Slot);
			Viewer->setCollisionShape(new btSphereShape(ViewerRadius / 100));
		}

		void Update(uint32 Slot, FVector Position)
		{
			ViewerObject** Viewer = Viewers.Find(Slot);
			btVector3 pos(Position.X / 100, Position.Y / 100, Position.Z / 100);
			if (Nodes[Slot] != nullptr)
			{
				Nodes[Slot]->setWorldTransform(btTransform(btMatrix3x3::getIdentity(), pos));
				if (Viewer)
				{
					(*Viewer)->setWorldTransform(btTransform(btMatrix3x3::getIdentity(), pos));
				}
			}
			else
			{
				Nodes[Slot] = new btCollisionObject;
				Nodes[Slot]->setCollisionShape(&DefaultShape);
				Nodes[Slot]->setUserIndex(Slot);
				Nodes[Slot]->setWorldTransform(btTransform(btMatrix3x3::getIdentity(), pos));
				World.addCollisionObject(Nodes[Slot], 1, 2);
				if (Viewer)
				{
					(*Viewer)->setWorldTransform(btTransform(btMatrix3x3::getIdentity(), pos));
					World.addCollisionObject(*Viewer, 2, 1);
				}
			}
		}

		void SwapEntries(int32 Slot1, int32 Slot2)
		{
			Swap(Nodes[Slot1], Nodes[Slot2]);
			Nodes[Slot1]->setUserIndex(Slot1);
			Nodes[Slot2]->setUserIndex(Slot2);

			ViewerObject** Viewer1Ptr = Viewers.Find(Slot1);
			ViewerObject* Viewer1 = Viewer1Ptr ? *Viewer1Ptr : nullptr;
			if (Viewer1)
			{
				Viewers.Remove(Slot1);
				Viewer1->setUserIndex(Slot2);
			}
			ViewerObject** Viewer2Ptr = Viewers.Find(Slot2);
			ViewerObject* Viewer2 = Viewer2Ptr ? *Viewer2Ptr : nullptr;
			if (Viewer2)
			{
				Viewers.Remove(Slot2);
				Viewer2->setUserIndex(Slot1);
				Viewers.Add(Slot1, Viewer2);
			}
			if (Viewer1)
			{
				Viewers.Add(Slot2, Viewer1);
			}
		}

		void MoveEntry(int32 SlotSrc, int32 SlotDest)
		{
			//TODO
			checkNoEntry();
			//Nodes[SlotSrc], Nodes[SlotDest];
			//
			//ViewerObject** Viewer = Viewers.Find(SlotSrc);
			//if (Viewer)
			//{
			//	(*Viewer)->setUserIndex(SlotDest);
			//	Viewers.Add(SlotDest, *Viewer);
			//	Viewers.Remove(SlotSrc);
			//}
		}

		void Remove()
		{
			uint32 LastSlot = Nodes.Num() - 1;

			World.removeCollisionObject(Nodes.Last());

			delete Nodes.Last();
			Nodes.Pop();

			ViewerObject** Viewer = Viewers.Find(LastSlot);
			if (Viewer)
			{
				World.removeCollisionObject(*Viewer);
				delete (*Viewer)->getCollisionShape();
				delete (*Viewer);
				Viewers.Remove(LastSlot);
			}
		}

		btDefaultCollisionConfiguration CollisionConfig;
		btCustomDispatcher Dispatcher;
		btHashed32OverlappingPairCache PairCache;
		btDbvtBroadphase Broadphase;
		btCollisionWorld World;
		btGhostPairCallback GhostCB;
		btSphereShape DefaultShape;

		TArray<btCollisionObject*> Nodes;
		TMap<uint32, ViewerObject*> Viewers;
	};

} // namespace SpatialGDK

#endif
