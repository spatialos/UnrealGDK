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
| ![#008000](https://placehold.it/15/008000/000000?text=+) FText | Yes | yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FString | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is POD | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is Stably named UObject | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is a dynamically created actor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-Style array\[T\] where T is a UStruct with Net Serialize | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is C++ 11 style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is POD | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is Stably named UObject* | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is a Dynamically created actor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct with net serialise | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TArray\[T\] where T is UStruct and the UStruct contains an Array. | No | No | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is C++ 11 style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C++ 11 style enum | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where both T1 and T2 are POD | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 and T2 are UStruct | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 and T2 are UObject* | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TWeakObjPtr<T> where T is UObject* | Yes | No | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with POD members only | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a nested UStruct | Yes | Yes | --- | ---|
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a nested UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a dynamically created actor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with Netserialize | yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with C-style array | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct With TArray | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with C++ 11 style enum | Yes | YEs | --- | --- |


Const arguments
Array with struct containing array

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
| ![#008000](https://placehold.it/15/008000/000000?text=+) FText | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) FString | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array<T> where T is POD | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array<T> where T is Stably named UObject | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is a dynamically created actor | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is UStruct | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-Style array\[T\] where T is a UStruct with Net Serialize | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is Unreal style enum | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C-style array\[T\] where T is C++ 11 style enum | No | No | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is POD | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is Stably named UObject* | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is a Dynamically created actor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is UStruct with net serialise | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TArray\[T\] where T is UStruct and the UStruct contains an Array. | No | No | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) TArray\[T\] where T is C++ 11 style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) C++ 11 style enum | Yes | Yes | --- | --- |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where both T1 and T2 are POD | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 and T2 are UStruct | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TMap<T1, T2> where either T1 and T2 are UObject* | No | No | --- | Currently not tested |
| ![#800000](https://placehold.it/15/800000/000000?text=+) TWeakObjPtr<T> where T is UObject* | Yes | No | --- | Currently not tested |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with POD members only | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a nested UStruct | Yes | Yes | --- | ---|
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a nested UStruct | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with a dynamically created actor | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with Netserialize | yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with C-style array | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct With TArray | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with Unreal style enum | Yes | Yes | --- | --- |
| ![#008000](https://placehold.it/15/008000/000000?text=+) UStruct with C++ 11 style enum | Yes | YEs | --- | --- |
