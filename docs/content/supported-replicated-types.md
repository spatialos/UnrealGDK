> This [pre-alpha](https://docs.improbable.io/reference/13.1/shared/release-policy#maturity-stages) release of the SpatialOS Unreal GDK is for evaluation and feedback purposes only, with limited documentation - see the guidance on [Recommended use](../../README.md#recommended-use).

# Unreal GDK type replication support

## Supported replicated types

| Type | Supported in Unreal | Supported in the Unreal GDK | Test implementation | Notes |
| --- | --- | --- | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Int | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Int8 | Yes | Yes | --- | --- |
|![#008000](https://placehold.it/15/008000/000000?text=+) Int16 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Int32 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) int64 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint8 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint16 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint32 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint64 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) float | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) double | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) bool | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) byte | No | No | --- | Use uint8 instead |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TCHAR | No | No | --- | Use uint8 instead |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FName | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FText | Yes | Yes | --- | Basic implementation, currently it does not support localised strings. |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FString | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is POD | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is stably named UObject | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is a dynamically created AActor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-Style array\[T\] where T is a UStruct with NetSerialize | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is an enum class | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is POD | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is stably named UObject* | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is a dynamically created AActor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct with NetSerialize | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TArray\[T\] where T is UStruct and the UStruct contains an TArray. | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TArray\[T\] where T is UStruct and the UStruct contains an C-style Array. | No | No | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is an enum class | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Enum class | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where both T1 and T2 are POD | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 or T2 are UStruct | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 or T2 are UObject* | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TWeakObjPtr<T> where T is UObject* | Yes | Yes | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with POD members only | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a nested UStruct | Yes | Yes | --- | ---|
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a dynamically created AActor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct pointing to stably named UObject  | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with NetSerialize | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with C-style array | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct With TArray | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with an enum class | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UObject pointing to dynamically created AActor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UObject pointing to stably named UObject  | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) const UObject pointing to stably named UObject  | Yes | Yes | --- | --- |

## Supported RPC arguments

| Type | Supported in Unreal | Supported in the Unreal GDK | Test implementation | Notes |
| --- | --- | --- | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Int | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Int8 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Int16 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Int32 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) int64 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint8 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint16 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint32 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) uint64 | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) float | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) double | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) bool | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) byte | No | No | --- | Use uint8 instead |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TCHAR | No | No | --- | Use uint8 instead |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FName | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FText | Yes | Yes | --- | Basic implementation, currently it does not support localised strings. |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FString | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array<T> where T is POD | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array<T> where T is stably named UObject | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is a dynamically created AActor | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is UStruct | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-Style array\[T\] where T is a UStruct with NetSerialize | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is Unreal style enum | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is an enum class | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is POD | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is stably named UObject* | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is a dynamically created AActor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct with net serialise | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TArray\[T\] where T is UStruct and the UStruct contains an TArray. | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TArray\[T\] where T is UStruct and the UStruct contains an C-style Array. | No | No | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is an enum class | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Enum class | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where both T1 and T2 are POD | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 or T2 are UStruct | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 or T2 are UObject* | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TWeakObjPtr<T> where T is UObject* | Yes | Yes | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with POD members only | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a nested UStruct | Yes | Yes | --- | ---|
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a dynamically created AActor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct pointing to stably named UObject  | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with NetSerialize | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with C-style array | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct With TArray | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with an enum class | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UObject pointing to dynamically created AActor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UObject pointing to stably named UObject  | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) const UObject pointing to stably named UObject  | Yes | Yes | --- | --- |
