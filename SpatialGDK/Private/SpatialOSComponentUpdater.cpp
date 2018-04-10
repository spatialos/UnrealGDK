// // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

// #include "SpatialOSComponentUpdater.h"

// #include "EntityRegistry.h"
// #include "SpatialOSComponent.h"

// void USpatialOSComponentUpdater::UpdateComponents(UEntityRegistry* Registry, float DeltaSeconds)
// {
//   if (Registry == nullptr)
//   {
//     return;
//   }

//   for (auto& Elem : Registry->EntityComponentCache)
//   {
//     if (Elem.Key == nullptr || Elem.Key->IsPendingKill())
//     {
//       continue;
//     }

//     for (auto& Component : Elem.Value.Components)
//     {
//       if (Component != nullptr)
//       {
//         Component->TriggerAutomaticComponentUpdate(DeltaSeconds);
//       }
//     }
//   }
// }