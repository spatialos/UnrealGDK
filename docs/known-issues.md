> This [pre-alpha](https://docs.improbable.io/reference/13.1/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use]({{urlRoot}}/index#recommended-use)

# Unreal GDK known issues

Known issue = any major user-facing bug or lack of user-facing feature that:
1. diverges from vanilla Unreal design or implementation (e.g. ordering of reliable RPCs), **OR**
1. diverges from user expectations from a SpatialOS project (e.g. interacting across worker boundaries)

| Issue | Date added | Ticket | Workaround? |
|-------|-------------------|--------|-------------|-------|
| Order of reliable RPCs is not respected in case of unresolved UObject* parameter. | 2018-06-14 | [UNR-363](https://improbableio.atlassian.net/browse/UNR-336) | Potential user-code workaround to explicitly enforce the order of RPCs. |
| Project build `PCHUsageMode` must be set to `UseExplicitOrSharedPCHs`. | 2018-06-27 |[UNR-354](https://improbableio.atlassian.net/browse/UNR-354)| |
| Adaptive Unity builds are not supported. | 2018-07-28 | | Disable `bUseAdaptiveUnityBuild` following [Epic’s documentation](https://docs.unrealengine.com/en-US/Programming/UnrealBuildSystem/Configuration). |
| Dynamic component replication. | 2018-07-03 | [UNR-366](https://improbableio.atlassian.net/browse/UNR-366) | Mimic behavior with static components. |
| Components with replicated sub-objects fail. | 2018-07-03 |[UNR-417](https://improbableio.atlassian.net/browse/UNR-417) | Make all components top level. |
| You can't have multiple replicated components of the same type on the same Actor. | 2018-07-03 | [UNR-416](https://improbableio.atlassian.net/browse/UNR-416) | None |
| The [Interop Code Generator]({{urlRoot}}/content/interop.md) is not optimized. | 2018-07-13 | | None | 
| User code must be IWYU-compliant, meaning you must import all the headers you use.| 2018-07-13 |[UNR-380](https://improbableio.atlassian.net/browse/UNR-380) | None |
| Compilation error for RPCs that take structs with private members. | 2018-07-16 | [UNR-144](https://improbableio.atlassian.net/browse/UNR-144) | Add the type binding as a friend class. |
|Interop codegen fails when the destination file is locked. The workflow is less than ideal as you can't run it again until you restart the Unreal Editor. | 2018-07-16 | [UNR-350](https://improbableio.atlassian.net/browse/UNR-350) | Ensure destination files/folders are unlocked. |
| Deleting entities directly from the Inspector causes undefined behavior. | 2018-07-17 | [UNR-425](https://improbableio.atlassian.net/projects/UNR/issues/UNR-425) | None |
| Having generated files in the symlinked folder `Source/SpatialGDK` means you can’t work in two projects at the same time using the same clone of the SpatialOS Unreal GDK. | 2018-07-23 | [UNR-458](https://improbableio.atlassian.net/browse/UNR-458) | Clone the Unreal GDK separately for each project. |
| We don't support listen servers. | 2018-07-30 | | Use dedicated servers instead. |
| Stably-named replicated actors cannot be referred to by their path after transitioning between servers | 2018-08-10 | [UNR-473](https://improbableio.atlassian.net/projects/UNR/issues/UNR-473) | None |
