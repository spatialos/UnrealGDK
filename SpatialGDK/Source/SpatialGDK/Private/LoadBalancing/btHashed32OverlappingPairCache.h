#pragma once

#include <BulletCollision/BroadphaseCollision/btOverlappingPairCache.h>

ATTRIBUTE_ALIGNED16(class)
btHashed32OverlappingPairCache : public btOverlappingPairCache
{
	btBroadphasePairArray m_overlappingPairArray;
	btOverlapFilterCallback* m_overlapFilterCallback;

protected:
	btAlignedObjectArray<int> m_hashTable;
	btAlignedObjectArray<int> m_next;
	btOverlappingPairCallback* m_ghostPairCallback;

public:
	BT_DECLARE_ALIGNED_ALLOCATOR();

	btHashed32OverlappingPairCache();
	virtual ~btHashed32OverlappingPairCache();

	void removeOverlappingPairsContainingProxy(btBroadphaseProxy * proxy, btDispatcher * dispatcher);

	virtual void* removeOverlappingPair(btBroadphaseProxy * proxy0, btBroadphaseProxy * proxy1, btDispatcher * dispatcher);

	SIMD_FORCE_INLINE bool needsBroadphaseCollision(btBroadphaseProxy * proxy0, btBroadphaseProxy * proxy1) const
	{
		if (m_overlapFilterCallback)
			return m_overlapFilterCallback->needBroadphaseCollision(proxy0, proxy1);

		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

		return collides;
	}

	// Add a pair and return the new pair. If the pair already exists,
	// no new pair is created and the old one is returned.
	virtual btBroadphasePair* addOverlappingPair(btBroadphaseProxy * proxy0, btBroadphaseProxy * proxy1)
	{
		if (!needsBroadphaseCollision(proxy0, proxy1))
			return 0;

		return internalAddPair(proxy0, proxy1);
	}

	void cleanProxyFromPairs(btBroadphaseProxy * proxy, btDispatcher * dispatcher);

	virtual void processAllOverlappingPairs(btOverlapCallback*, btDispatcher * dispatcher);

	virtual void processAllOverlappingPairs(btOverlapCallback * callback, btDispatcher * dispatcher, const struct btDispatcherInfo& dispatchInfo);

	virtual btBroadphasePair* getOverlappingPairArrayPtr()
	{
		return &m_overlappingPairArray[0];
	}

	const btBroadphasePair* getOverlappingPairArrayPtr() const
	{
		return &m_overlappingPairArray[0];
	}

	btBroadphasePairArray& getOverlappingPairArray()
	{
		return m_overlappingPairArray;
	}

	const btBroadphasePairArray& getOverlappingPairArray() const
	{
		return m_overlappingPairArray;
	}

	void cleanOverlappingPair(btBroadphasePair & pair, btDispatcher * dispatcher);

	btBroadphasePair* findPair(btBroadphaseProxy * proxy0, btBroadphaseProxy * proxy1);

	int GetCount() const { return m_overlappingPairArray.size(); }
	//	btBroadphasePair* GetPairs() { return m_pairs; }

	btOverlapFilterCallback* getOverlapFilterCallback()
	{
		return m_overlapFilterCallback;
	}

	void setOverlapFilterCallback(btOverlapFilterCallback * callback)
	{
		m_overlapFilterCallback = callback;
	}

	int getNumOverlappingPairs() const
	{
		return m_overlappingPairArray.size();
	}

private:
	btBroadphasePair* internalAddPair(btBroadphaseProxy * proxy0, btBroadphaseProxy * proxy1);

	void growTables();

	SIMD_FORCE_INLINE bool equalsPair(const btBroadphasePair& pair, int proxyId1, int proxyId2)
	{
		return pair.m_pProxy0->getUid() == proxyId1 && pair.m_pProxy1->getUid() == proxyId2;
	}

	SIMD_FORCE_INLINE unsigned int getHash(unsigned int proxyId1, unsigned int proxyId2)
	{
		return HashCombine(proxyId1, proxyId2);
	}

	SIMD_FORCE_INLINE btBroadphasePair* internalFindPair(btBroadphaseProxy * proxy0, btBroadphaseProxy * proxy1, int hash)
	{
		int proxyId1 = proxy0->getUid();
		int proxyId2 = proxy1->getUid();
#if 0  // wrong, 'equalsPair' use unsorted uids, copy-past devil striked again. Nat.
		if (proxyId1 > proxyId2) 
			btSwap(proxyId1, proxyId2);
#endif

		int index = m_hashTable[hash];

		while (index != BT_NULL_PAIR && equalsPair(m_overlappingPairArray[index], proxyId1, proxyId2) == false)
		{
			index = m_next[index];
		}

		if (index == BT_NULL_PAIR)
		{
			return NULL;
		}

		btAssert(index < m_overlappingPairArray.size());

		return &m_overlappingPairArray[index];
	}

	virtual bool hasDeferredRemoval()
	{
		return false;
	}

	virtual void setInternalGhostPairCallback(btOverlappingPairCallback * ghostPairCallback)
	{
		m_ghostPairCallback = ghostPairCallback;
	}

	virtual void sortOverlappingPairs(btDispatcher * dispatcher);
};
