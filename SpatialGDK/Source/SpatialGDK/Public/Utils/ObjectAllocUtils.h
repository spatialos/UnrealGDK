// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

/**
 * Helper to declare that a type cannot be allocated on the heap (which remembers the allocation size)
 * (it can still be stored in an array though)
 * This should be enough to prevent types from requiring polymorphic destruction (calling delete on a base pointer).
 * (if they become polymorphic, it should only be in a context where
 * the code responsible for storing it knows its final type, hence not needing a virtual dtor)
 */
class FNoHeapAllocation
{
	void* operator new(size_t) = delete;
	void* operator new[](size_t) = delete;
	void operator delete(void*) = delete;
	void operator delete[](void*) = delete;

public:
	void* operator new(size_t, void* Placement) { return Placement; }
};

/**
 * Helper to declare that a type can only be stored on an auto-storage
 * (stack most of the time because global storage should be discouraged)
 */
class FStackOnly : FNoHeapAllocation
{
	void* operator new(size_t, void*) = delete;
};
