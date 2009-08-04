// Copyright 2009 Olivier Gillet. All rights reserved
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Resources manager. Support for lookup of values/strings in tables. Since
// one might not want this functionality and just use the plain program memory
// read/write function, an alias for a stripped down version without string
// table lookup is provided (SimpleResourcesManager).

#ifndef HARDWARE_RESOURCES_RESOURCES_MANAGER_H_
#define HARDWARE_RESOURCES_RESOURCES_MANAGER_H_

#include "hardware/base/base.h"

#ifdef __TEST__
#include <string.h>
#else
#include <avr/pgmspace.h>
#endif  // __TEST__

namespace hardware_resources {

template<const prog_char** strings, const prog_uint16_t** lookup_tables>
struct ResourcesTables {
  static inline const prog_char** string_table() { return strings; }
  static inline const prog_uint16_t** lookup_table_table() {
      return lookup_tables; }
};

struct NoResourcesTables {
  static inline const prog_char** string_table() { return NULL; }
  static inline const prog_uint16_t** lookup_table_table() { return NULL; }
};

template<typename ResourceId = uint8_t, typename Tables = NoResourcesTables>
struct ResourcesManager {
  static inline void LoadStringResource(ResourceId resource, char* buffer,
                                        uint8_t buffer_size) {
    if (!Tables::string_table()) {
      return;
    }
#ifdef __TEST__
    strncpy(buffer, Tables::string_table()[resource], buffer_size);
#else
    char* address = (char*)(pgm_read_word(&(Tables::string_table()[resource])));
    strncpy_P(buffer, address, buffer_size);
#endif  // __TEST__
  }
  
  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(ResourceId resource, IndexType i) {
    if (!Tables::lookup_table_table()) {
      return 0;
    };
#ifdef __TEST__
    return ResultType(Tables::lookup_table_table()[resource][i]);
#else
    uint16_t* address = (uint16_t*)(
    pgm_read_word(&(Tables::lookup_table_table()[resource])));
    return ResultType(pgm_read_word(address + i));
#endif  // __TEST__
  }
  
  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(const prog_char* p, IndexType i) {
#ifdef __TEST__
    return ResultType(p[i]);
#else
    return ResultType(pgm_read_byte(p + i));
#endif  // __TEST__
  }

  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(const prog_uint8_t* p, IndexType i) {
#ifdef __TEST__
    return ResultType(p[i]);
#else
    return ResultType(pgm_read_byte(p + i));
#endif  // __TEST__
  }

  
  template<typename ResultType, typename IndexType>
  static inline ResultType Lookup(const prog_uint16_t* p, IndexType i) {
#ifdef __TEST__
    return ResultType(p[i]);
#else
    return ResultType(pgm_read_word(p + i));
#endif  // __TEST__
  }

  template<typename T>
  static void Load(const prog_char* p, uint8_t i, T* destination) {
    uint8_t* destination_ptr = (uint8_t*)destination;
    const uint8_t* source = (const uint8_t*)p;
    source += sizeof(T) * i;
    for (int i = 0; i < sizeof(T); ++i) {
#ifdef __TEST__
      *destination_ptr++ = *source++;
#else
      *destination_ptr++ = pgm_read_byte(source++);
#endif  // __TEST__
    }
  }
};

typedef ResourcesManager<> SimpleResourcesManager;

}  // namespace hardware_resources

#endif  // HARDWARE_RESOURCES_RESOURCES_MANAGER_H_