// Automatically generated from runtime/ArrayPrototype.cpp using /JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue arrayTableValues[22] = {
   { "toString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncToString), (intptr_t)0 },
   { "toLocaleString", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncToLocaleString), (intptr_t)0 },
   { "concat", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncConcat), (intptr_t)1 },
   { "join", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncJoin), (intptr_t)1 },
   { "pop", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncPop), (intptr_t)0 },
   { "push", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncPush), (intptr_t)1 },
   { "reverse", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncReverse), (intptr_t)0 },
   { "shift", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncShift), (intptr_t)0 },
   { "slice", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSlice), (intptr_t)2 },
   { "sort", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSort), (intptr_t)1 },
   { "splice", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSplice), (intptr_t)2 },
   { "unshift", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncUnShift), (intptr_t)1 },
   { "every", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncEvery), (intptr_t)1 },
   { "forEach", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncForEach), (intptr_t)1 },
   { "some", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncSome), (intptr_t)1 },
   { "indexOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncIndexOf), (intptr_t)1 },
   { "lastIndexOf", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncLastIndexOf), (intptr_t)1 },
   { "filter", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncFilter), (intptr_t)1 },
   { "reduce", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncReduce), (intptr_t)1 },
   { "reduceRight", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncReduceRight), (intptr_t)1 },
   { "map", DontEnum|Function, (intptr_t)static_cast<NativeFunction>(arrayProtoFuncMap), (intptr_t)1 },
   { 0, 0, 0, 0 }
};

extern JSC_CONST_HASHTABLE HashTable arrayTable =
    { 65, 63, arrayTableValues, 0 };
} // namespace
