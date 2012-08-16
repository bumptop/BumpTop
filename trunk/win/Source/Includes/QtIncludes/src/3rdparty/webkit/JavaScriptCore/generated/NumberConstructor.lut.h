// Automatically generated from runtime/NumberConstructor.cpp using /JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue numberTableValues[6] = {
   { "NaN", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorNaNValue), (intptr_t)0 },
   { "NEGATIVE_INFINITY", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorNegInfinity), (intptr_t)0 },
   { "POSITIVE_INFINITY", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorPosInfinity), (intptr_t)0 },
   { "MAX_VALUE", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorMaxValue), (intptr_t)0 },
   { "MIN_VALUE", DontEnum|DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(numberConstructorMinValue), (intptr_t)0 },
   { 0, 0, 0, 0 }
};

extern JSC_CONST_HASHTABLE HashTable numberTable =
    { 16, 15, numberTableValues, 0 };
} // namespace
