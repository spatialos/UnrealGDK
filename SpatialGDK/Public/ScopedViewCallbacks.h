// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKViewTypes..h"
#include "SpatialGDKWorkerTypes.h"

namespace improbable
{
namespace unreal
{
namespace callbacks
{
/**
 * Automatically deregister callbacks from a View upon destruction of the
 * object. Callbacks can be deregistered manually using DeregisterAllCallbacks().
 */
class FScopedViewCallbacks
{
public:
  FScopedViewCallbacks()
  {
    View.Reset();
  }

  FScopedViewCallbacks(const TWeakPtr<SpatialOSView>& InView) : View(InView)
  {
  }

  ~FScopedViewCallbacks()
  {
    Reset();
  }

  /**
  * Initialise the FScopedViewCallbacks. Calling Init() more than once results in an error.
  */
  void Init(const TWeakPtr<SpatialOSView>& InView)
  {
    auto LockedView = View.Pin();
    checkf(!LockedView.IsValid(), TEXT("Attempting to call Init() more than once."));
    View = InView;
  }

  /**
  * Reset the FScopedViewCallbacks to its initial state. Init() may be called again after this
  * method is called.
  */
  void Reset()
  {
    DeregisterAllCallbacks();
    View.Reset();
  }

  /**
  * Manually deregister all callbacks.
  */
  void DeregisterAllCallbacks()
  {
    auto LockedView = View.Pin();
    if (LockedView.IsValid())
    {
      for (auto handle : Handles)
      {
        LockedView->Remove(handle);
      }
    }
  }

  /** Add a handle that will be tracked and removed upon destruction. */
  void Add(uint64 handle)
  {
    checkSlow(Handles.Find(handle) == INDEX_NONE);
    Handles.Add(handle);
  }

  /**
   * Remove a handle from tracking.
   * It is up to the caller to remove it from the View if it is still valid.
   */
  void Remove(uint64 handle)
  {
    auto numRemoved = Handles.RemoveSwap(handle);
    check(numRemoved == 1);
  }

private:
  TWeakPtr<SpatialOSView> View;
  TArray<uint64> Handles;
};
}  // ::callbacks
}  // ::unreal
}  // ::improbable
