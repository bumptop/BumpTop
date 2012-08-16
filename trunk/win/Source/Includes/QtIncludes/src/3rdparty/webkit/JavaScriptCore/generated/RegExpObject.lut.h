// Automatically generated from runtime/RegExpObject.cpp using /JavaScriptCore/create_hash_table. DO NOT EDIT!

#include "Lookup.h"

namespace JSC {

static const struct HashTableValue regExpTableValues[6] = {
   { "global", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectGlobal), (intptr_t)0 },
   { "ignoreCase", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectIgnoreCase), (intptr_t)0 },
   { "multiline", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectMultiline), (intptr_t)0 },
   { "source", DontDelete|ReadOnly|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectSource), (intptr_t)0 },
   { "lastIndex", DontDelete|DontEnum, (intptr_t)static_cast<PropertySlot::GetValueFunc>(regExpObjectLastIndex), (intptr_t)setRegExpObjectLastIndex },
   { 0, 0, 0, 0 }
};

extern JSC_CONST_HASHTABLE HashTable regExpTable =
    { 17, 15, regExpTableValues, 0 };
} // namespace
