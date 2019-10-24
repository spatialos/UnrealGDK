/* Copyright (c) Improbable Worlds Ltd, All Rights Reserved. */
#ifndef WORKER_SDK_C_INCLUDE_IMPROBABLE_C_SCHEMA_H
#define WORKER_SDK_C_INCLUDE_IMPROBABLE_C_SCHEMA_H

/* This file defines an interface for manipulating objects corresponding to the types defined in the
 * SpatialOS schema in a dynamic way.
 *
 * Schema_Object is the main type for data manipulation, and roughly-speaking corresponds to an
 * instance of a "type" as defined in schema. Each Schema_Object is owned by a "root" schema type
 * instance, of which there are four: Schema_CommandRequest, Schema_CommandResponse,
 * Schema_ComponentData, and Schema_ComponentUpdate.
 *
 * Each field defined in schema has a _field ID_, a  _type_ and an _arity_. For each type, there is
 * a family of functions that can be used to read and write fields of that type for a particular
 * field ID inside a Schema_Object. The mapping from schema type to function family is given below:
 *
 *      .schema type | function family
 * ------------------|----------------
 *             int32 | Int32
 *             int64 | Int64
 *            uint32 | Uint32
 *            uint64 | Uint64
 *            sint32 | Sint32
 *            sint64 | Sint64
 *           fixed32 | Fixed32
 *           fixed64 | Fixed64
 *          sfixed32 | Sfixed32
 *          sfixed64 | Sfixed64
 *              bool | Bool
 *             float | Float
 *            double | Double
 *            string | Bytes
 *          EntityId | EntityId (alias for Int64)
 *             bytes | Bytes
 * user-defined enum | Enum (alias for Uint32)
 * user-defined type | Object
 *
 * The arity of a field is either singular, option, or list. The same function family can be used
 * for manipulating fields of any arity: a singular field is simply a field whose ID occurs exactly
 * once; an option field is a field whose ID occurs zero or one times; and a list field is a field
 * whose ID occurs any number of times.
 *
 * Therefore, typically, where X is the function family, we use the Schema_GetX and Schema_AddX
 * functions to read and write singular fields; the Schema_GetXCount, Schema_GetX and Schema_AddX
 * functions to read and write option fields, and the Schema_GetXCount, Schema_IndexX and
 * Schema_AddX functions to read and write list fields. However, these functions are all
 * interopable: internally, Schema_GetX just retrieves the last occurence of the given field ID, for
 * instance.
 *
 * A Map field is an Object that contains a list of Object fields. Each object represents an entry
 * in the map, and has the key under field ID 1 (SCHEMA_MAP_KEY_FIELD_ID) and the value under field
 * ID 2 (SCHEMA_MAP_VALUE_FIELD_ID).
 *
 * An Entity field is a set of Component data. It is represented as an Object with one field per
 * component data. A given component is stored with a field Id equal to the component Id.
 *
 * It is the responsibility of the user to ensure that Schema_Objects are accessed and mutated in a
 * way consistent with the schema definitions of their corresponding types. Typically, this is done
 * by writing a custom code generator for the schema AST.
 */

#include <stdint.h>

/* Map fields are represented in schema as a list of pairs of key and value. Each entry in the map
 * is an object field with field ID corresponding to the map's field ID, and each object should have
 * a key field with ID SCHEMA_MAP_KEY_FIELD_ID and a value field with ID
 * SCHEMA_MAP_VALUE_FIELD_ID. */
#define SCHEMA_MAP_KEY_FIELD_ID 1
#define SCHEMA_MAP_VALUE_FIELD_ID 2

#ifdef WORKER_DLL
#ifdef WORKER_DLL_EXPORT
#if defined(_MSC_VER) || defined(__ORBIS__)
#define WORKER_API __declspec(dllexport)
#else /* defined(_MSC_VER) || defined(__ORBIS__) */
#define WORKER_API __attribute__((visibility("default")))
#endif /* defined(_MSC_VER) || defined(__ORBIS__) */
#else  /* WORKER_DLL_EXPORT */
#if defined(_MSC_VER) || defined(__ORBIS__)
#define WORKER_API __declspec(dllimport)
#else /* defined(_MSC_VER) || defined(__ORBIS__) */
#define WORKER_API
#endif /* defined(_MSC_VER) || defined(__ORBIS__) */
#endif /* WORKER_DLL_EXPORT */
#else  /* WORKER_DLL */
#define WORKER_API
#endif /* WORKER_DLL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef uint32_t Schema_FieldId;
typedef int64_t Schema_EntityId;

/* Root schema type instances. */
struct Schema_CommandRequest;
struct Schema_CommandResponse;
struct Schema_ComponentData;
struct Schema_ComponentUpdate;
/** An object, roughly corresponding to an instance of a "type" as defined in schema. */
struct Schema_Object;
typedef struct Schema_CommandRequest Schema_CommandRequest;
typedef struct Schema_CommandResponse Schema_CommandResponse;
typedef struct Schema_ComponentData Schema_ComponentData;
typedef struct Schema_ComponentUpdate Schema_ComponentUpdate;
typedef struct Schema_Object Schema_Object;

/**
 * Allocate a command request schema type instance. The component ID should be as defined in
 * the schema, and the command_index should be the 1-based position of the command in the order
 * the commands appear in component in the schema.
 */
WORKER_API Schema_CommandRequest* Schema_CreateCommandRequest(Schema_FieldId component_id,
                                                              Schema_FieldId command_index);
/**
 * Performs a deep copy of the source command request and returns the new copy.
 */
WORKER_API Schema_CommandRequest* Schema_CopyCommandRequest(const Schema_CommandRequest* source);
/** Free the resources associated with a command request schema type instance. */
WORKER_API void Schema_DestroyCommandRequest(Schema_CommandRequest* request);
/** Get the component ID of a command request. */
WORKER_API Schema_FieldId Schema_GetCommandRequestComponentId(const Schema_CommandRequest* request);
/** Get the 1-based position of the command in the order the commands appear in the schema. */
WORKER_API Schema_FieldId
Schema_GetCommandRequestCommandIndex(const Schema_CommandRequest* request);
/** Get the command request as a Schema_Object. */
WORKER_API Schema_Object* Schema_GetCommandRequestObject(Schema_CommandRequest* request);

/**
 * Allocate a command response schema type instance. The component ID should be as defined in
 * the schema, and the command_index should be the 1-based position of the command in the order
 * the commands appear in the component in schema.
 */
WORKER_API Schema_CommandResponse* Schema_CreateCommandResponse(Schema_FieldId component_id,
                                                                Schema_FieldId command_index);
/**
 * Performs a deep copy of the source command response and returns the new copy.
 */
WORKER_API Schema_CommandResponse* Schema_CopyCommandResponse(const Schema_CommandResponse* source);
/** Free the resources associated with a command response schema type instance. */
WORKER_API void Schema_DestroyCommandResponse(Schema_CommandResponse* response);
/** Get the component ID of a command response. */
WORKER_API Schema_FieldId
Schema_GetCommandResponseComponentId(const Schema_CommandResponse* request);
/** Get the 1-based position of the command in the order the commands appear in the schema. */
WORKER_API Schema_FieldId
Schema_GetCommandResponseCommandIndex(const Schema_CommandResponse* request);
/** Get the command response as a Schema_Object. */
WORKER_API Schema_Object* Schema_GetCommandResponseObject(Schema_CommandResponse* response);

/** Allocate a component data snapshot schema type instance. */
WORKER_API Schema_ComponentData* Schema_CreateComponentData(Schema_FieldId component_id);
/**
 * Performs a deep copy of the source component data and returns the new copy.
 */
WORKER_API Schema_ComponentData* Schema_CopyComponentData(const Schema_ComponentData* source);
/** Free the resources associated with a component data snapshot schema type instance. */
WORKER_API void Schema_DestroyComponentData(Schema_ComponentData* data);
/** Get the component ID of a component data snapshot. */
WORKER_API Schema_FieldId Schema_GetComponentDataComponentId(const Schema_ComponentData* data);
/** Get the command data snapshot as a Schema_Object. */
WORKER_API Schema_Object* Schema_GetComponentDataFields(Schema_ComponentData* data);

/** Allocate a component update schema type instance. */
WORKER_API Schema_ComponentUpdate* Schema_CreateComponentUpdate(Schema_FieldId component_id);
/**
 * Performs a deep copy of the source component update and returns the new copy.
 */
WORKER_API Schema_ComponentUpdate* Schema_CopyComponentUpdate(const Schema_ComponentUpdate* source);
/** Free the resources associated with a component update schema type instance. */
WORKER_API void Schema_DestroyComponentUpdate(Schema_ComponentUpdate* update);
/** Get the component ID of a component update. */
WORKER_API Schema_FieldId
Schema_GetComponentUpdateComponentId(const Schema_ComponentUpdate* update);
/**
 * Get an object representing the non-event fields in a component update. This object should be used
 * as if it had one field for each field in the component, whose type corresponds to the type of the
 * field as defined in schema. Note that when an option, list or map field in a component is set to
 * the empty value, it will not / should not appear here. Instead, use
 * Schema_IndexComponentUpdateClearedField and related functions.
 */
WORKER_API Schema_Object* Schema_GetComponentUpdateFields(Schema_ComponentUpdate* update);
/**
 * Get an object representing the event fields in a component update. This object should be used
 * as if it had one field for each event in the component. Each field behaves like a list (may have
 * multiple instances of the same event), and the field ID of an event is its 1-based position in
 * the order the events appear in the component in the schema.
 */
WORKER_API Schema_Object* Schema_GetComponentUpdateEvents(Schema_ComponentUpdate* update);
/**
 * Clears the list of fields that this update sets to the empty value (for option, list and map
 * fields in a component).
 */
WORKER_API void Schema_ClearComponentUpdateClearedFields(Schema_ComponentUpdate* update);
/**
 * Checks whether this updates sets an option, list of map field in a component to the empty value.
 */
WORKER_API uint8_t Schema_IsComponentUpdateFieldCleared(Schema_ComponentUpdate* update,
                                                        Schema_FieldId field_id);
/**
 * Specifies that this update sets an option, list or map field in a component to the empty
 * value.
 */
WORKER_API void Schema_AddComponentUpdateClearedField(Schema_ComponentUpdate* update,
                                                      Schema_FieldId field_id);
/**
 * Returns the number of option, list and map fields in a component that this update sets to the
 * empty value.
 */
WORKER_API uint32_t
Schema_GetComponentUpdateClearedFieldCount(const Schema_ComponentUpdate* update);
/**
 * Returns the field ID of an option, list or map field which is set to the empty value by this
 * update.
 */
WORKER_API Schema_FieldId
Schema_IndexComponentUpdateClearedField(const Schema_ComponentUpdate* update, uint32_t index);
/**
 * Returns all field IDs of option, list, or map fields which are set to the empty value by this
 * component. The output_array should have space for
 * Schema_GetComponentUpdateClearedFieldCount(update) field IDs.
 */
WORKER_API void Schema_GetComponentUpdateClearedFieldList(const Schema_ComponentUpdate* update,
                                                          Schema_FieldId* output_array);

/**
 * Merges a component update 'update' into an at-rest component data object 'target_data'. This is
 * semantically equivalent to a component update being "applied" to the at-rest component data.
 *
 * This function will mutate the component data object by appending the bytes representation of the
 * update to the end of the component data. This is efficient because if the component update is
 * small, it wouldn't need to reshuffle any fields under the hood, but will have unbounded memory
 * growth (proportional to the serialized component update after each function call).
 *
 * To avoid mutating the data and having unbounded memory growth, use this function in combination
 * with Schema_CopyComponentData to copy the component data first. Note that calling
 * Schema_CopyComponentData on a component data will only require storage for the serialized size of
 * the component data in the returned object, hence why memory growth is no longer unbounded when
 * this is used.
 *
 * Returns 1 if success, 0 if there was a failure. Call
 * `Schema_GetError(Schema_GetComponentDataFields(target_data))` to get the error message.
 */
WORKER_API uint8_t Schema_ApplyComponentUpdateToData(const Schema_ComponentUpdate* update,
                                                     Schema_ComponentData* target_data);
/**
 * Merges a component update 'update' into another component update object 'target_update'. This is
 * semantically equivalent to combining two component updates into a single component update such
 * that, when applied to some at-rest component data, it will be semantically equivalent to applying
 * 'target_update' followed by 'update'. 'target_update' will be mutated to contain the resulting
 * combined update.
 *
 * Unlike Schema_ApplyComponentUpdateToData, this function will not have unbounded memory growth. To
 * avoid mutating 'target_update', use this function in combination with Schema_CopyComponentUpdate
 * to create a copy which will contain the resulting merged update.
 *
 * 'update' will be empty after this operation, so ensure to call Schema_DestroyComponentUpdate to
 * avoid leaking memory.
 *
 * Returns 1 if success, 0 if there was a failure. Call
 * `Schema_GetError(Schema_GetComponentUpdateFields(target_update))` to get the error message.
 */
WORKER_API uint8_t Schema_MergeComponentUpdateIntoUpdate(Schema_ComponentUpdate* update,
                                                         Schema_ComponentUpdate* target_update);

/** Completely clears all fields in the given object. */
WORKER_API void Schema_Clear(Schema_Object* object);
/** Completely clears the given field ID in the given object. */
WORKER_API void Schema_ClearField(Schema_Object* object, Schema_FieldId field_id);
/**
 * Copies all fields from `src` to `dst`. The copy is shallow; changes made to object fields in the
 * source will also be reflected in the copied fields.
 *
 * If `src == dst`, or if the objects are not associated with the same root schema type instance, no
 * operation is performed.
 */
WORKER_API void Schema_ShallowCopy(const Schema_Object* src, Schema_Object* dst);
/**
 * Copies over a field from `src` to `dst`. If multiple fields with the given field_id exist all
 * are copied. The copy is shallow; changes made to object fields in the source will also be
 * reflected in the copied fields.
 *
 * If `src == dst`, or if the objects are not associated with the same root schema type instance, no
 * operation is performed.
 */
WORKER_API void Schema_ShallowCopyField(const Schema_Object* src, Schema_Object* dst,
                                        Schema_FieldId field_id);

/**
 * Allocates an orphaned Schema_Object in memory owned by the given Schema_Object instance. The
 * returned object is owned by the associated schema type instance, but is not reachable from any
 * other object. The memory is freed by a call to Schema_Destroy.
 */
WORKER_API Schema_Object* Schema_AllocateObject(const Schema_Object* object);

/**
 * Allocates a buffer of the specified length in bytes from memory owned by the given Schema_Object
 * instance. The memory is freed by a call to Schema_Destroy.
 *
 * Note: this is useful for allocating memory that must live as long as the root schema type
 * instance, for example to pass to Schema_MergeFromBuffer.
 */
WORKER_API uint8_t* Schema_AllocateBuffer(Schema_Object* object, uint32_t length);
/**
 * Merges the given buffer into the given object, appending all fields. This function
 * can fail; if the return value is zero, call Schema_GetError to obtain an error string.
 *
 * Note: the provided buffer is not copied, and must live as long as the root schema type instance.
 */
WORKER_API uint8_t Schema_MergeFromBuffer(Schema_Object* object, const uint8_t* buffer,
                                          uint32_t length);
/** Computes the serialized length of the given Schema_Object. */
WORKER_API uint32_t Schema_GetWriteBufferLength(const Schema_Object* object);
/**
 * Serializes the given object into the provided buffer, which _must_ have space at
 * least equal to the length returned by Schema_WriteBufferLength. This function can
 * fail; if the return value is zero, call Schema_GetError to obtain an error string.
 *
 * WriteToBuffer re-calculates the size of the underlying Schema_Object and is therefore less
 * performant than Schema_SerializeToBuffer. Schema_WriteToBuffer is deprecated and will be removed
 * in 14+ releases.
 *
 * `length` must equal the value returned by `Schema_GetWriteBufferLength`. Otherwise, the behavior
 * is undefined.
 */
WORKER_API uint8_t Schema_WriteToBuffer(const Schema_Object* object, uint8_t* buffer);
WORKER_API uint8_t Schema_SerializeToBuffer(const Schema_Object* object, uint8_t* buffer,
                                            uint32_t length);
/**
 * Obtains the most recent error encountered by any object associated with the given object. The
 * buffer is owned by the schema object that manages the memory of `object`, and may get replaced
 * after calling another Schema function. Returns `NULL` if no error has occurred within the given
 * object.
 */
WORKER_API const char* Schema_GetError(const Schema_Object* object);
/** Returns the number of unique field IDs used in the Schema_Object. */
WORKER_API uint32_t Schema_GetUniqueFieldIdCount(const Schema_Object* object);
/**
 * Returns the sorted list of unique field IDs used in the Schema_Object. The buffer parameter
 * must have space remaining for as many field IDs as indicated by Schema_GetUniqueFieldIdCount.
 */
WORKER_API void Schema_GetUniqueFieldIds(const Schema_Object* object, uint32_t* buffer);

/* Functions that append a single value to the set of fields present in an object. For any field ID,
 * these can be called repeatedly to construct a list of values, or mixed freely with the AddList
 * functions below; however, making a single call to an AddList function is the most efficient way
 * to construct a list of values. Note that, for best performance, fields should be added to the
 * object in field ID order. */
WORKER_API void Schema_AddFloat(Schema_Object* object, Schema_FieldId field_id, float value);
WORKER_API void Schema_AddDouble(Schema_Object* object, Schema_FieldId field_id, double value);
WORKER_API void Schema_AddBool(Schema_Object* object, Schema_FieldId field_id, uint8_t value);
WORKER_API void Schema_AddInt32(Schema_Object* object, Schema_FieldId field_id, int32_t value);
WORKER_API void Schema_AddInt64(Schema_Object* object, Schema_FieldId field_id, int64_t value);
WORKER_API void Schema_AddUint32(Schema_Object* object, Schema_FieldId field_id, uint32_t value);
WORKER_API void Schema_AddUint64(Schema_Object* object, Schema_FieldId field_id, uint64_t value);
WORKER_API void Schema_AddSint32(Schema_Object* object, Schema_FieldId field_id, int32_t value);
WORKER_API void Schema_AddSint64(Schema_Object* object, Schema_FieldId field_id, int64_t value);
WORKER_API void Schema_AddFixed32(Schema_Object* object, Schema_FieldId field_id, uint32_t value);
WORKER_API void Schema_AddFixed64(Schema_Object* object, Schema_FieldId field_id, uint64_t value);
WORKER_API void Schema_AddSfixed32(Schema_Object* object, Schema_FieldId field_id, int32_t value);
WORKER_API void Schema_AddSfixed64(Schema_Object* object, Schema_FieldId field_id, int64_t value);
WORKER_API void Schema_AddEntityId(Schema_Object* object, Schema_FieldId field_id,
                                   Schema_EntityId value);
WORKER_API void Schema_AddEnum(Schema_Object* object, Schema_FieldId field_id, uint32_t value);
WORKER_API void Schema_AddBytes(Schema_Object* object, Schema_FieldId field_id,
                                const uint8_t* buffer, uint32_t length);
WORKER_API Schema_Object* Schema_AddObject(Schema_Object* object, Schema_FieldId field_id);

/* Functions that append a list of primitive values for a particular field ID to an object. Note
 * that, for best performance, fields should be added to the object in field ID order.
 *
 * Note: no copy of the data is made. The source data must live as long as the root schema type
 * instance. */
WORKER_API void Schema_AddFloatList(Schema_Object* object, Schema_FieldId field_id,
                                    const float* values, uint32_t count);
WORKER_API void Schema_AddDoubleList(Schema_Object* object, Schema_FieldId field_id,
                                     const double* values, uint32_t count);
WORKER_API void Schema_AddBoolList(Schema_Object* object, Schema_FieldId field_id,
                                   const uint8_t* values, uint32_t count);
WORKER_API void Schema_AddInt32List(Schema_Object* object, Schema_FieldId field_id,
                                    const int32_t* values, uint32_t count);
WORKER_API void Schema_AddInt64List(Schema_Object* object, Schema_FieldId field_id,
                                    const int64_t* values, uint32_t count);
WORKER_API void Schema_AddUint32List(Schema_Object* object, Schema_FieldId field_id,
                                     const uint32_t* values, uint32_t count);
WORKER_API void Schema_AddUint64List(Schema_Object* object, Schema_FieldId field_id,
                                     const uint64_t* values, uint32_t count);
WORKER_API void Schema_AddSint32List(Schema_Object* object, Schema_FieldId field_id,
                                     const int32_t* values, uint32_t count);
WORKER_API void Schema_AddSint64List(Schema_Object* object, Schema_FieldId field_id,
                                     const int64_t* values, uint32_t count);
WORKER_API void Schema_AddFixed32List(Schema_Object* object, Schema_FieldId field_id,
                                      const uint32_t* values, uint32_t count);
WORKER_API void Schema_AddFixed64List(Schema_Object* object, Schema_FieldId field_id,
                                      const uint64_t* values, uint32_t count);
WORKER_API void Schema_AddSfixed32List(Schema_Object* object, Schema_FieldId field_id,
                                       const int32_t* values, uint32_t count);
WORKER_API void Schema_AddSfixed64List(Schema_Object* object, Schema_FieldId field_id,
                                       const int64_t* values, uint32_t count);
WORKER_API void Schema_AddEntityIdList(Schema_Object* object, Schema_FieldId field_id,
                                       const Schema_EntityId* values, uint32_t count);
WORKER_API void Schema_AddEnumList(Schema_Object* object, Schema_FieldId field_id,
                                   const uint32_t* values, uint32_t count);

/* Functions that determine the number of occurrences of a particular field ID in an object.
 *
 * Note that, for best performance, fields should be accessed in field ID order. */
WORKER_API uint32_t Schema_GetFloatCount(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetDoubleCount(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetBoolCount(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetInt32Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetInt64Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetUint32Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetUint64Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetSint32Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetSint64Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetFixed32Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetFixed64Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetSfixed32Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetSfixed64Count(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetEntityIdCount(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetEnumCount(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetBytesCount(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetObjectCount(const Schema_Object* object, Schema_FieldId field_id);

/* Functions that access a single value for a particular field ID in an object. Boolean values are
 * guaranteed to be either 0 or 1. These functions assume the field is non-repeated, i.e. if the
 * field appears multiple times in the object, only the last field will be accessed (equivalent to
 * `Schema_Index*(object, field_id, Schema_Get*Count(object, field_id) - 1)`).
 *
 * If the field does not exist, a default value is returned; call the corresponding GetCount
 * function above to determine if the field is present.
 *
 * Note that, for best performance, fields should be accessed in field ID order. */
WORKER_API float Schema_GetFloat(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API double Schema_GetDouble(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint8_t Schema_GetBool(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API int32_t Schema_GetInt32(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API int64_t Schema_GetInt64(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetUint32(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint64_t Schema_GetUint64(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API int32_t Schema_GetSint32(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API int64_t Schema_GetSint64(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetFixed32(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint64_t Schema_GetFixed64(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API int32_t Schema_GetSfixed32(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API int64_t Schema_GetSfixed64(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API Schema_EntityId Schema_GetEntityId(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetEnum(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API uint32_t Schema_GetBytesLength(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API const uint8_t* Schema_GetBytes(const Schema_Object* object, Schema_FieldId field_id);
WORKER_API Schema_Object* Schema_GetObject(Schema_Object* object, Schema_FieldId field_id);

/* Functions that access a value by index for a particular field ID in an object. Boolean values are
 * guaranteed to be either 0 or 1.
 *
 * If the index doesn't exist for the given field, a default is returned; call
 * the corresponding GetCount function above to to determine if the total number of fields.
 *
 * Note that, for best performance, fields should be accessed in field ID and index order. */
WORKER_API float Schema_IndexFloat(const Schema_Object* object, Schema_FieldId field_id,
                                   uint32_t index);
WORKER_API double Schema_IndexDouble(const Schema_Object* object, Schema_FieldId field_id,
                                     uint32_t index);
WORKER_API uint8_t Schema_IndexBool(const Schema_Object* object, Schema_FieldId field_id,
                                    uint32_t index);
WORKER_API int32_t Schema_IndexInt32(const Schema_Object* object, Schema_FieldId field_id,
                                     uint32_t index);
WORKER_API int64_t Schema_IndexInt64(const Schema_Object* object, Schema_FieldId field_id,
                                     uint32_t index);
WORKER_API uint32_t Schema_IndexUint32(const Schema_Object* object, Schema_FieldId field_id,
                                       uint32_t index);
WORKER_API uint64_t Schema_IndexUint64(const Schema_Object* object, Schema_FieldId field_id,
                                       uint32_t index);
WORKER_API int32_t Schema_IndexSint32(const Schema_Object* object, Schema_FieldId field_id,
                                      uint32_t index);
WORKER_API int64_t Schema_IndexSint64(const Schema_Object* object, Schema_FieldId field_id,
                                      uint32_t index);
WORKER_API uint32_t Schema_IndexFixed32(const Schema_Object* object, Schema_FieldId field_id,
                                        uint32_t index);
WORKER_API uint64_t Schema_IndexFixed64(const Schema_Object* object, Schema_FieldId field_id,
                                        uint32_t index);
WORKER_API int32_t Schema_IndexSfixed32(const Schema_Object* object, Schema_FieldId field_id,
                                        uint32_t index);
WORKER_API int64_t Schema_IndexSfixed64(const Schema_Object* object, Schema_FieldId field_id,
                                        uint32_t index);
WORKER_API Schema_EntityId Schema_IndexEntityId(const Schema_Object* object,
                                                Schema_FieldId field_id, uint32_t index);
WORKER_API uint32_t Schema_IndexEnum(const Schema_Object* object, Schema_FieldId field_id,
                                     uint32_t index);
WORKER_API uint32_t Schema_IndexBytesLength(const Schema_Object* object, Schema_FieldId field_id,
                                            uint32_t index);
WORKER_API const uint8_t* Schema_IndexBytes(const Schema_Object* object, Schema_FieldId field_id,
                                            uint32_t index);
WORKER_API Schema_Object* Schema_IndexObject(Schema_Object* object, Schema_FieldId field_id,
                                             uint32_t index);

/* Functions that copy the complete list of values for a particular field ID in an object. Boolean
 * values are guaranteed to be either 0 or 1. The provided output array must have space for at least
 * as many elements as returned by the corresponding GetCount function.
 *
 * Note that, for best performance, fields should be accessed in field ID and index order. */
WORKER_API void Schema_GetFloatList(const Schema_Object* object, Schema_FieldId field_id,
                                    float* output_array);
WORKER_API void Schema_GetDoubleList(const Schema_Object* object, Schema_FieldId field_id,
                                     double* output_array);
WORKER_API void Schema_GetBoolList(const Schema_Object* object, Schema_FieldId field_id,
                                   uint8_t* output_array);
WORKER_API void Schema_GetInt32List(const Schema_Object* object, Schema_FieldId field_id,
                                    int32_t* output_array);
WORKER_API void Schema_GetInt64List(const Schema_Object* object, Schema_FieldId field_id,
                                    int64_t* output_array);
WORKER_API void Schema_GetUint32List(const Schema_Object* object, Schema_FieldId field_id,
                                     uint32_t* output_array);
WORKER_API void Schema_GetUint64List(const Schema_Object* object, Schema_FieldId field_id,
                                     uint64_t* output_array);
WORKER_API void Schema_GetSint32List(const Schema_Object* object, Schema_FieldId field_id,
                                     int32_t* output_array);
WORKER_API void Schema_GetSint64List(const Schema_Object* object, Schema_FieldId field_id,
                                     int64_t* output_array);
WORKER_API void Schema_GetFixed32List(const Schema_Object* object, Schema_FieldId field_id,
                                      uint32_t* output_array);
WORKER_API void Schema_GetFixed64List(const Schema_Object* object, Schema_FieldId field_id,
                                      uint64_t* output_array);
WORKER_API void Schema_GetSfixed32List(const Schema_Object* object, Schema_FieldId field_id,
                                       int32_t* output_array);
WORKER_API void Schema_GetSfixed64List(const Schema_Object* object, Schema_FieldId field_id,
                                       int64_t* output_array);
WORKER_API void Schema_GetEntityIdList(const Schema_Object* object, Schema_FieldId field_id,
                                       Schema_EntityId* output_array);
WORKER_API void Schema_GetEnumList(const Schema_Object* object, Schema_FieldId field_id,
                                   uint32_t* output_array);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WORKER_SDK_C_INCLUDE_IMPROBABLE_C_SCHEMA_H */
