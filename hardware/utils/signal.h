// Copyright 2009 Olivier Gillet. All rights reserved
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Signal processing functions for the synthesis engine.

#ifndef HARDWARE_UTILS_SIGNAL_H_
#define HARDWARE_UTILS_SIGNAL_H_

#include "hardware/base/base.h"

namespace hardware_utils {
  
struct Signal {
  static inline int16_t Clip(int16_t value, int16_t min, int16_t max) {
    return value < min ? min : (value > max ? max : value);
  }
  #ifdef __FAST_SIGNAL_PROCESSING__
  static inline uint8_t Clip8(int16_t value) {
    uint8_t result;
    asm(
      "mov %0, %A1"   "\n\t"  // by default, copy the value.
      "or %B1, %B1"   "\n\t"  // load H to set flags.
      "brpl .+4"      "\n\t"  // if positive, skip
      "ldi %0, 0"     "\n\t"  // set to 0.
      "rjmp .+4"      "\n\t"  // and jump.
      "breq .+2"      "\n\t"  // if null, skip
      "ldi %0, 255"   "\n\t"  // set to 255
      : "=r" (result)
      : "a" (value)
    );
    return result;  
  }
  static inline int8_t SignedClip8(int16_t value) {
    return Clip8(value + 128) + 128;  
  }
  static inline uint8_t Mix(uint8_t a, uint8_t b, uint8_t balance) {
    uint16_t sum;
    asm(
      "mul %2, %1"      "\n\t"  // b * balance
      "movw %A3, r0"    "\n\t"  // to sum
      "com %1"          "\n\t"  // 255 - balance
      "mul %0, %1"      "\n\t"  // a * (255 - balance)
      "com %1"          "\n\t"  // reset balance to its previous value
      "add %A3, r0"     "\n\t"  // add to sum L
      "adc %B3, r1"     "\n\t"  // add to sum H
      "eor r1, r1"      "\n\t"  // reset r1 after multiplication
      "mov %0, %B3"     "\n\t"  // use H as output
      : "+r" (a)
      : "a" (balance), "a" (b), "a" (sum)
      );
    return a;
  }
  static inline uint8_t MulScale8(uint8_t a, uint8_t b) {
    uint8_t result;
    asm(
      "mul %1, %2"      "\n\t"
      "mov %0, r1"      "\n\t"
      "eor r1, r1"      "\n\t"
      : "=r" (result)
      : "a" (a), "a" (b)
    );
    return result;
  }
  static inline int8_t SignedMulScale8(int8_t a, uint8_t b) {
    uint8_t result;
    asm(
      "mulsu %1, %2"    "\n\t"
      "mov %0, r1"      "\n\t"
      "eor r1, r1"      "\n\t"
      : "=r" (result)
      : "a" (a), "a" (b)
    );
    return result;
  }
  static inline int16_t SignedMulScale4(int8_t a, uint8_t b) {
    int16_t result;
    asm(
      "mulsu %1, %2"    "\n\t"
      "asr r1"          "\n\t"
      "ror r0"          "\n\t"
      "asr r1"          "\n\t"
      "ror r0"          "\n\t"
      "asr r1"          "\n\t"
      "ror r0"          "\n\t"
      "asr r1"          "\n\t"
      "ror r0"          "\n\t"
      "mov %B0, r1"     "\n\t"
      "eor r1, r1"      "\n\t"
      "mov %A0, r0"     "\n\t"
      : "=r" (result)
      : "a" (a), "a" (b)
    );
    return result;
  }
  #else
  static inline uint8_t Clip8(int16_t value) {
    return value < 0 ? 0 : (value > 255 ? 255 : value);
  }
  static inline int8_t SignedClip8(int16_t value) {
    return value < -128 ? -128 : (value > 127 ? 127 : value);
  }
  static inline uint8_t Mix(uint8_t a, uint8_t b, uint8_t balance) {
    uint8_t cmp_balance = (255 - balance);
    uint16_t sum = a * cmp_balance + b * balance;
    return sum >> 8;
  }
  static inline uint8_t MulScale8(uint8_t a, uint8_t b) {
    return a * b >> 8;
  }
  static inline int8_t SignedMulScale8(int8_t a, uint8_t b) {
    return int8_t(a) * b >> 8;
  }
  static inline int16_t SignedMulScale4(int8_t a, uint8_t b) {
    return int16_t(int8_t(a) * uint8_t(b)) >> 4;
  }
  #endif  // __FAST_SIGNAL_PROCESSING__
};

}  // namespace hardware_utils

#endif  // HARDWARE_UTILS_SIGNAL_H_