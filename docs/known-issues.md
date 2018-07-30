## Unreal GDK Known Issues

Known issue = any major user-facing bug or lack of user-facing feature that:
1. diverges from vanilla Unreal design or implementation (e.g. ordering of reliable RPCs), **OR**
1. diverges from user expectations from a SpatialOS project (e.g. interacting across worker boundaries)

| Issue | Date added | Ticket | Workaround? | Done? |
|-------|-------------------|--------|-------------|-------|
| Order of reliable RPCs is not respected in case of unresolved UObject* parameter. | 2018-06-14 | [UNR-363](https://improbableio.atlassian.net/browse/UNR-336) | Potential user-code workaround. | No|
| Reliable multicast RPCs aren’t supported. | 2018-06-14 | | Ask yourself if you really need a reliable multicast RPC. | No |
| Project build `PCHUsageMode` must be set to `UseExplicitOrSharedPCHs`. | 2018-06-27 |[UNR-354](https://improbableio.atlassian.net/browse/UNR-354)| | No |
| Adaptive Unity builds are not supported. | 2018-07-28 | | Disable `bUseAdaptiveUnityBuild` following [Epic’s documentation](https://docs.unrealengine.com/en-US/Programming/UnrealBuildSystem/Configuration). | No |
| When working on a blueprint class, we need to specify the blueprint class in the DefaultEditorSpatialGDk.ini. E.g SampleGameCharacterBP, but we actually need to add SampleGameCharacterBP_C, the _C is an Unreal workaround. | 2018-06-27 | [UNR-363](https://improbableio.atlassian.net/browse/UNR-363) | Append `_C` to blueprint classes names in the `.ini` file. | No |
| Dynamic component replication. | 2018-07-03 | [UNR-366](https://improbableio.atlassian.net/browse/UNR-366) | Mimic behavior with static components (not really a workaround). | No |
| Components with replicated sub-objects fail. | 2018-07-03 |[UNR-417](https://improbableio.atlassian.net/browse/UNR-417) | Make all components top-level. | No |
| Multiple replicated components of the same type. | 2018-07-03 | [UNR-416](https://improbableio.atlassian.net/browse/UNR-416) | | No |
| The [Interop Code Generator](./content/interop.md) is not optimised. | 2018-07-13 | | None | No |
| User code must be IWYU-compliant, meaning you must import all the headers you use.| 2018-07-13 |[UNR-380](https://improbableio.atlassian.net/browse/UNR-380) | No | No |
| The SpatialOS Unreal GDK currently launches the `GameDefaultMap` instead of the `EditorDefaultMap` in PIE windows. | 2018-07-16 | [UNR-227](https://improbableio.atlassian.net/browse/UNR-227) | Duplicated EditorDefaultMap into GameDefaultMap. | No |
| Compilation error for RPCs that take structs with private members. | 2018-07-16 | [UNR-144](https://improbableio.atlassian.net/browse/UNR-144) | Add the type binding as a friend class. | No |
|Interop codegen fails when the destination file is locked. The workflow is less than ideal as you can't run it again until you restart the Unreal Editor. | 2018-07-16 | [UNR-350](https://improbableio.atlassian.net/browse/UNR-350) | Ensure destination files/folders are unlocked. | No |
| Deleting entities directly from the Inspector causes undefined behavior. | 2018-07-17 | [UNR-425](https://improbableio.atlassian.net/projects/UNR/issues/UNR-425) | | No |
| Interop code generation stalls if its child processes produce console output during execution. |2018-07-19 | [UNR-422](https://improbableio.atlassian.net/browse/UNR-422) | Disable verbose output. This stops interop code generation from stalling, but you won't receive error messages if it fails. | No |
| Having generated files in the symlinked folder `Source/SpatialGDK` means you can’t work in two projects at the same time using the same clone of the SpatialOS Unreal GDK. | 2018-07-23 | [UNR-458](https://improbableio.atlassian.net/browse/UNR-458) | Separate UnrealGDK checkout for each project. | No |
| We don't support listen servers. | 2018-07-30 | | Use dedicated servers instead. | No |