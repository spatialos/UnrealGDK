
#include "btHashed32OverlappingPairCache.h"

#include <BulletCollision/BroadphaseCollision/btDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <LinearMath/btAabbUtil2.h>

btHashed32OverlappingPairCache::btHashed32OverlappingPairCache() : m_overlapFilterCallback(0),
															   m_ghostPairCallback(0)
{
	int initialAllocatedSize = 2;
	m_overlappingPairArray.reserve(initialAllocatedSize);
	growTables();
}

btHashed32OverlappingPairCache::~btHashed32OverlappingPairCache()
{
}

void btHashed32OverlappingPairCache::cleanOverlappingPair(btBroadphasePair& pair, btDispatcher* dispatcher)
{
	if (pair.m_algorithm && dispatcher)
	{
		{
			pair.m_algorithm->~btCollisionAlgorithm();
			dispatcher->freeCollisionAlgorithm(pair.m_algorithm);
			pair.m_algorithm = 0;
		}
	}
}

void btHashed32OverlappingPairCache::cleanProxyFromPairs(btBroadphaseProxy* proxy, btDispatcher* dispatcher)
{
	class CleanPairCallback : public btOverlapCallback
	{
		btBroadphaseProxy* m_cleanProxy;
		btOverlappingPairCache* m_pairCache;
		btDispatcher* m_dispatcher;

	public:
		CleanPairCallback(btBroadphaseProxy* cleanProxy, btOverlappingPairCache* pairCache, btDispatcher* dispatcher)
			: m_cleanProxy(cleanProxy),
			  m_pairCache(pairCache),
			  m_dispatcher(dispatcher)
		{
		}
		virtual bool processOverlap(btBroadphasePair& pair)
		{
			if ((pair.m_pProxy0 == m_cleanProxy) ||
				(pair.m_pProxy1 == m_cleanProxy))
			{
				m_pairCache->cleanOverlappingPair(pair, m_dispatcher);
			}
			return false;
		}
	};

	CleanPairCallback cleanPairs(proxy, this, dispatcher);

	processAllOverlappingPairs(&cleanPairs, dispatcher);
}

void btHashed32OverlappingPairCache::removeOverlappingPairsContainingProxy(btBroadphaseProxy* proxy, btDispatcher* dispatcher)
{
	class RemovePairCallback : public btOverlapCallback
	{
		btBroadphaseProxy* m_obsoleteProxy;

	public:
		RemovePairCallback(btBroadphaseProxy* obsoleteProxy)
			: m_obsoleteProxy(obsoleteProxy)
		{
		}
		virtual bool processOverlap(btBroadphasePair& pair)
		{
			return ((pair.m_pProxy0 == m_obsoleteProxy) ||
					(pair.m_pProxy1 == m_obsoleteProxy));
		}
	};

	RemovePairCallback removeCallback(proxy);

	processAllOverlappingPairs(&removeCallback, dispatcher);
}

btBroadphasePair* btHashed32OverlappingPairCache::findPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1)
{
	if (proxy0->m_uniqueId > proxy1->m_uniqueId)
		btSwap(proxy0, proxy1);
	int proxyId1 = proxy0->getUid();
	int proxyId2 = proxy1->getUid();

	/*if (proxyId1 > proxyId2)
		btSwap(proxyId1, proxyId2);*/

	int hash = static_cast<int>(getHash(static_cast<unsigned int>(proxyId1), static_cast<unsigned int>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));

	if (hash >= m_hashTable.size())
	{
		return NULL;
	}

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

//#include <stdio.h>

void btHashed32OverlappingPairCache::growTables()
{
	int newCapacity = m_overlappingPairArray.capacity();

	if (m_hashTable.size() < newCapacity)
	{
		//grow hashtable and next table
		int curHashtableSize = m_hashTable.size();

		m_hashTable.resize(newCapacity);
		m_next.resize(newCapacity);

		int i;

		for (i = 0; i < newCapacity; ++i)
		{
			m_hashTable[i] = BT_NULL_PAIR;
		}
		for (i = 0; i < newCapacity; ++i)
		{
			m_next[i] = BT_NULL_PAIR;
		}

		for (i = 0; i < curHashtableSize; i++)
		{
			const btBroadphasePair& pair = m_overlappingPairArray[i];
			int proxyId1 = pair.m_pProxy0->getUid();
			int proxyId2 = pair.m_pProxy1->getUid();
			/*if (proxyId1 > proxyId2) 
				btSwap(proxyId1, proxyId2);*/
			int hashValue = static_cast<int>(getHash(static_cast<unsigned int>(proxyId1), static_cast<unsigned int>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));  // New hash value with new mask
			m_next[i] = m_hashTable[hashValue];
			m_hashTable[hashValue] = i;
		}
	}
}

btBroadphasePair* btHashed32OverlappingPairCache::internalAddPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1)
{
	if (proxy0->m_uniqueId > proxy1->m_uniqueId)
		btSwap(proxy0, proxy1);
	int proxyId1 = proxy0->getUid();
	int proxyId2 = proxy1->getUid();

	/*if (proxyId1 > proxyId2) 
		btSwap(proxyId1, proxyId2);*/

	int hash = static_cast<int>(getHash(static_cast<unsigned int>(proxyId1), static_cast<unsigned int>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));  // New hash value with new mask

	btBroadphasePair* pair = internalFindPair(proxy0, proxy1, hash);
	if (pair != NULL)
	{
		return pair;
	}
	/*for(int i=0;i<m_overlappingPairArray.size();++i)
		{
		if(	(m_overlappingPairArray[i].m_pProxy0==proxy0)&&
			(m_overlappingPairArray[i].m_pProxy1==proxy1))
			{
			printf("Adding duplicated %u<>%u\r\n",proxyId1,proxyId2);
			internalFindPair(proxy0, proxy1, hash);
			}
		}*/
	int count = m_overlappingPairArray.size();
	int oldCapacity = m_overlappingPairArray.capacity();
	void* mem = &m_overlappingPairArray.expandNonInitializing();

	//this is where we add an actual pair, so also call the 'ghost'
	if (m_ghostPairCallback)
		m_ghostPairCallback->addOverlappingPair(proxy0, proxy1);

	int newCapacity = m_overlappingPairArray.capacity();

	if (oldCapacity < newCapacity)
	{
		growTables();
		//hash with new capacity
		hash = static_cast<int>(getHash(static_cast<unsigned int>(proxyId1), static_cast<unsigned int>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));
	}

	pair = new (mem) btBroadphasePair(*proxy0, *proxy1);
	//	pair->m_pProxy0 = proxy0;
	//	pair->m_pProxy1 = proxy1;
	pair->m_algorithm = 0;
	pair->m_internalTmpValue = 0;

	m_next[count] = m_hashTable[hash];
	m_hashTable[hash] = count;

	return pair;
}

void* btHashed32OverlappingPairCache::removeOverlappingPair(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1, btDispatcher* dispatcher)
{
	if (proxy0->m_uniqueId > proxy1->m_uniqueId)
		btSwap(proxy0, proxy1);
	int proxyId1 = proxy0->getUid();
	int proxyId2 = proxy1->getUid();

	/*if (proxyId1 > proxyId2)
		btSwap(proxyId1, proxyId2);*/

	int hash = static_cast<int>(getHash(static_cast<unsigned int>(proxyId1), static_cast<unsigned int>(proxyId2)) & (m_overlappingPairArray.capacity() - 1));

	btBroadphasePair* pair = internalFindPair(proxy0, proxy1, hash);
	if (pair == NULL)
	{
		return 0;
	}

	cleanOverlappingPair(*pair, dispatcher);

	void* userData = pair->m_internalInfo1;

	btAssert(pair->m_pProxy0->getUid() == proxyId1);
	btAssert(pair->m_pProxy1->getUid() == proxyId2);

	int pairIndex = int(pair - &m_overlappingPairArray[0]);
	btAssert(pairIndex < m_overlappingPairArray.size());

	// Remove the pair from the hash table.
	int index = m_hashTable[hash];
	btAssert(index != BT_NULL_PAIR);

	int previous = BT_NULL_PAIR;
	while (index != pairIndex)
	{
		previous = index;
		index = m_next[index];
	}

	if (previous != BT_NULL_PAIR)
	{
		btAssert(m_next[previous] == pairIndex);
		m_next[previous] = m_next[pairIndex];
	}
	else
	{
		m_hashTable[hash] = m_next[pairIndex];
	}

	// We now move the last pair into spot of the
	// pair being removed. We need to fix the hash
	// table indices to support the move.

	int lastPairIndex = m_overlappingPairArray.size() - 1;

	if (m_ghostPairCallback)
		m_ghostPairCallback->removeOverlappingPair(proxy0, proxy1, dispatcher);

	// If the removed pair is the last pair, we are done.
	if (lastPairIndex == pairIndex)
	{
		m_overlappingPairArray.pop_back();
		return userData;
	}

	// Remove the last pair from the hash table.
	const btBroadphasePair* last = &m_overlappingPairArray[lastPairIndex];
	/* missing swap here too, Nat. */
	int lastHash = static_cast<int>(getHash(static_cast<unsigned int>(last->m_pProxy0->getUid()), static_cast<unsigned int>(last->m_pProxy1->getUid())) & (m_overlappingPairArray.capacity() - 1));

	index = m_hashTable[lastHash];
	btAssert(index != BT_NULL_PAIR);

	previous = BT_NULL_PAIR;
	while (index != lastPairIndex)
	{
		previous = index;
		index = m_next[index];
	}

	if (previous != BT_NULL_PAIR)
	{
		btAssert(m_next[previous] == lastPairIndex);
		m_next[previous] = m_next[lastPairIndex];
	}
	else
	{
		m_hashTable[lastHash] = m_next[lastPairIndex];
	}

	// Copy the last pair into the remove pair's spot.
	m_overlappingPairArray[pairIndex] = m_overlappingPairArray[lastPairIndex];

	// Insert the last pair into the hash table
	m_next[pairIndex] = m_hashTable[lastHash];
	m_hashTable[lastHash] = pairIndex;

	m_overlappingPairArray.pop_back();

	return userData;
}
//#include <stdio.h>
#include "LinearMath/btQuickprof.h"
void btHashed32OverlappingPairCache::processAllOverlappingPairs(btOverlapCallback* callback, btDispatcher* dispatcher)
{
	BT_PROFILE("btHashed32OverlappingPairCache::processAllOverlappingPairs");
	int i;

	//	printf("m_overlappingPairArray.size()=%d\n",m_overlappingPairArray.size());
	for (i = 0; i < m_overlappingPairArray.size();)
	{
		btBroadphasePair* pair = &m_overlappingPairArray[i];
		if (callback->processOverlap(*pair))
		{
			removeOverlappingPair(pair->m_pProxy0, pair->m_pProxy1, dispatcher);
		}
		else
		{
			i++;
		}
	}
}

struct MyPairIndex
{
	int m_orgIndex;
	int m_uidA0;
	int m_uidA1;
};

class MyPairIndeSortPredicate
{
public:
	bool operator()(const MyPairIndex& a, const MyPairIndex& b) const
	{
		const int uidA0 = a.m_uidA0;
		const int uidB0 = b.m_uidA0;
		const int uidA1 = a.m_uidA1;
		const int uidB1 = b.m_uidA1;
		return uidA0 > uidB0 || (uidA0 == uidB0 && uidA1 > uidB1);
	}
};

void btHashed32OverlappingPairCache::processAllOverlappingPairs(btOverlapCallback* callback, btDispatcher* dispatcher, const struct btDispatcherInfo& dispatchInfo)
{
	if (dispatchInfo.m_deterministicOverlappingPairs)
	{
		btBroadphasePairArray& pa = getOverlappingPairArray();
		btAlignedObjectArray<MyPairIndex> indices;
		{
			BT_PROFILE("sortOverlappingPairs");
			indices.resize(pa.size());
			for (int i = 0; i < indices.size(); i++)
			{
				const btBroadphasePair& p = pa[i];
				const int uidA0 = p.m_pProxy0 ? p.m_pProxy0->m_uniqueId : -1;
				const int uidA1 = p.m_pProxy1 ? p.m_pProxy1->m_uniqueId : -1;

				indices[i].m_uidA0 = uidA0;
				indices[i].m_uidA1 = uidA1;
				indices[i].m_orgIndex = i;
			}
			indices.quickSort(MyPairIndeSortPredicate());
		}
		{
			BT_PROFILE("btHashed32OverlappingPairCache::processAllOverlappingPairs");
			int i;
			for (i = 0; i < indices.size();)
			{
				btBroadphasePair* pair = &pa[indices[i].m_orgIndex];
				if (callback->processOverlap(*pair))
				{
					removeOverlappingPair(pair->m_pProxy0, pair->m_pProxy1, dispatcher);
				}
				else
				{
					i++;
				}
			}
		}
	}
	else
	{
		processAllOverlappingPairs(callback, dispatcher);
	}
}

void btHashed32OverlappingPairCache::sortOverlappingPairs(btDispatcher* dispatcher)
{
	///need to keep hashmap in sync with pair address, so rebuild all
	btBroadphasePairArray tmpPairs;
	int i;
	for (i = 0; i < m_overlappingPairArray.size(); i++)
	{
		tmpPairs.push_back(m_overlappingPairArray[i]);
	}

	for (i = 0; i < tmpPairs.size(); i++)
	{
		removeOverlappingPair(tmpPairs[i].m_pProxy0, tmpPairs[i].m_pProxy1, dispatcher);
	}

	for (i = 0; i < m_next.size(); i++)
	{
		m_next[i] = BT_NULL_PAIR;
	}

	tmpPairs.quickSort(btBroadphasePairSortPredicate());

	for (i = 0; i < tmpPairs.size(); i++)
	{
		addOverlappingPair(tmpPairs[i].m_pProxy0, tmpPairs[i].m_pProxy1);
	}
}
