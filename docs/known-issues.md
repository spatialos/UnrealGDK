> This [pre-alpha](https://docs.improbable.io/reference/13.1/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use](../README.md#recommended-use).

# Unreal GDK known issues

Known issue = any major user-facing bug or lack of user-facing feature that:
1. diverges from vanilla Unreal design or implementation (e.g. ordering of reliable RPCs), **OR**
1. diverges from user expectations from a SpatialOS project (e.g. interacting across worker boundaries)

| Issue | Date added | Ticket | Workaround? | Done? |
|-------|-------------------|--------|-------------|-------|
| Order of reliable RPCs is not respected in case of unresolved UObject* parameter. | 2018-06-14 | [UNR-363](https://improbableio.atlassian.net/browse/UNR-336) | Potential user-code workaround to explicitly enforce the order of RPCs. | No|
| Project build `PCHUsageMode` must be set to `UseExplicitOrSharedPCHs`. | 2018-06-27 |[UNR-354](https://improbableio.atlassian.net/browse/UNR-354)| | No |
| Adaptive Unity builds are not supported. | 2018-07-28 | | Disable `bUseAdaptiveUnityBuild` following [Epic’s documentation](https://docs.unrealengine.com/en-US/Programming/UnrealBuildSystem/Configuration). | No |
| Dynamic component replication. | 2018-07-03 | [UNR-366](https://improbableio.atlassian.net/browse/UNR-366) | Mimic behavior with static components. | No |
| Components with replicated sub-objects fail. | 2018-07-03 |[UNR-417](https://improbableio.atlassian.net/browse/UNR-417) | Make all components top level. | No |
| You can't have multiple replicated components of the same type on the same Actor. | 2018-07-03 | [UNR-416](https://improbableio.atlassian.net/browse/UNR-416) | | No |
| The [Interop Code Generator](./content/interop.md) is not optimized. | 2018-07-13 | | None | No |
| User code must be IWYU-compliant, meaning you must import all the headers you use.| 2018-07-13 |[UNR-380](https://improbableio.atlassian.net/browse/UNR-380) | No | No |
| The SpatialOS Unreal GDK currently launches the `GameDefaultMap` instead of the `EditorDefaultMap` in PIE windows. | 2018-07-16 | [UNR-227](https://improbableio.atlassian.net/browse/UNR-227) | Duplicated EditorDefaultMap into GameDefaultMap. | No |
| Compilation error for RPCs that take structs with private members. | 2018-07-16 | [UNR-144](https://improbableio.atlassian.net/browse/UNR-144) | Add the type binding as a friend class. | No |
|Interop codegen fails when the destination file is locked. The workflow is less than ideal as you can't run it again until you restart the Unreal Editor. | 2018-07-16 | [UNR-350](https://improbableio.atlassian.net/browse/UNR-350) | Ensure destination files/folders are unlocked. | No |
| Deleting entities directly from the Inspector causes undefined behavior. | 2018-07-17 | [UNR-425](https://improbableio.atlassian.net/projects/UNR/issues/UNR-425) | | No |
| Having generated files in the symlinked folder `Source/SpatialGDK` means you can’t work in two projects at the same time using the same clone of the SpatialOS Unreal GDK. | 2018-07-23 | [UNR-458](https://improbableio.atlassian.net/browse/UNR-458) | Clone the Unreal GDK separately for each project. | No |
| We don't support listen servers. | 2018-07-30 | | Use dedicated servers instead. | No |
| Attempting to find the bound class of a blueprint class' typebinding fails if the blueprint hasn't previously been loaded. | 2018-07-31 | [UNR-273](https://improbableio.atlassian.net/browse/UNR-273) | Add a reference to the blueprint in default level. | No
| GameState is sometimes not created/updating properly. In a new deployment this can cause characters to be in a T-pose. | 2018-07-31 | [UNR-475](https://improbableio.atlassian.net/browse/UNR-475) | Try reconnecting until it works. | No
| Stably-named replicated actors cannot be referred to by their path after transitioning between servers | 2018-08-06 | [UNR-473](https://improbableio.atlassian.net/projects/UNR/issues/UNR-473) |  | No
