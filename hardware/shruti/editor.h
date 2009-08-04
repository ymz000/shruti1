// Copyright 2009 Olivier Gillet. All rights reserved
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Parameter editor.

#ifndef HARDWARE_SHRUTI_EDITOR_H_
#define HARDWARE_SHRUTI_EDITOR_H_

#include <stdio.h>

#include "hardware/base/base.h"
#include "hardware/io/display.h"
#include "hardware/shruti/parameters.h"
#include "hardware/shruti/patch.h"
#include "hardware/shruti/resources.h"
#include "hardware/shruti/shruti.h"

using hardware_io::Display;
using hardware_io::kLcdWidth;
using hardware_io::kLcdNoCursor;

namespace hardware_shruti {

enum CurrentDisplayType {
  PAGE_TYPE_SUMMARY,
  PAGE_TYPE_DETAILS
};

enum Group {
  GROUP_OSC,
  GROUP_FILTER,
  GROUP_MOD,
  GROUP_PLAY,
  GROUP_LOAD_SAVE,
};

enum Page {
  PAGE_OSC_OSC_1,
  PAGE_OSC_OSC_2,
  PAGE_OSC_OSC_MIX,
  PAGE_FILTER_FILTER,
  PAGE_MOD_ENV,
  PAGE_MOD_LFO,
  PAGE_MOD_MATRIX,
  PAGE_PLAY_ARP,
  PAGE_PLAY_STEP_SEQUENCER,
  PAGE_PLAY_KBD,
  PAGE_LOAD_SAVE,
  PAGE_END = 255
};

enum Unit {
  UNIT_RAW_UINT8,
  UNIT_UINT8,
  UNIT_INT8,
  UNIT_BOOLEAN,
  UNIT_WAVEFORM,
  UNIT_LFO_WAVEFORM,
  UNIT_MODULATION_SOURCE,
  UNIT_MODULATION_DESTINATION,
  UNIT_PATTERN,
  UNIT_RAGA,
  UNIT_TEMPO_WITH_EXTERNAL_CLOCK,
};

enum Action {
  ACTION_LOAD,
  ACTION_EXIT,
  ACTION_SAVE
};

// We do not use enums here because they take 2 bytes, not 1.
typedef uint8_t ParameterGroup;
typedef uint8_t ParameterPage;
typedef uint8_t ParameterUnit;

static const uint8_t kNumPages = 11;
static const uint8_t kNumGroups = 6;
static const uint8_t kNumControllers = 4;

// Size (in char) of the display elements.
static const uint8_t kCaptionWidth = 10;
static const uint8_t kValueWidth = 6;
static const uint8_t kColumnWidth = 4;

struct ParameterDefinition {
  uint8_t id;
  uint8_t min_value;
  uint8_t max_value;
  ParameterPage page;
  ParameterUnit unit;
  ResourceId short_name;
  ResourceId long_name;
};

class Editor;

struct PageDefinition {
  ParameterPage id;
  ParameterGroup group;
  ResourceId name;
  void (Editor::*summary_page)();
  void (Editor::*details_page)();
  void (Editor::*input_handler)(uint8_t controller_index, uint16_t value);
};

class SynthesisEngine;

class Editor {
 public:
  Editor();
  void Init(Display* display);
  void ToggleGroup(ParameterGroup group);
  void CyclePage();

  void HandleInput(uint8_t controller_index, uint16_t value);
  void DisplaySummary();
  void DisplayDetails();
  void DisplaySplashScreen();
  
  void ResetPatch();
  inline ParameterPage current_page() { return current_page_; }
  inline uint8_t cursor() { return cursor_; }

 private:
  static const PageDefinition page_definition_[];

  // Parameter definitions are stored in program memory and need to be copied
  // in SRAM when read. This temporary storage space holds them.
  const ParameterDefinition& parameter_definition(uint8_t index);
  ParameterDefinition parameter_definition_;
  uint8_t parameter_definition_index_;
  uint8_t current_display_type_;  // 0 for summary, 1 for details.
  
  Display* display_;
  
  ParameterPage current_page_;
  ParameterPage last_visited_page_[kNumGroups];
  uint8_t current_controller_;
  uint8_t parameter_definition_offset_[kNumPages][kNumControllers];

  char line_buffer_[kLcdWidth + 1];
  
  void PrettyPrintParameterValue(const ParameterDefinition& parameter,
                                 char* buffer, uint8_t width);

  // Load/Save related stuff. Cursor is also used for the step sequencer, and
  // for storing the modulation matrix entry being edited. You're warned!
  uint16_t cursor_;
  uint8_t flip_;  // Used for blinking cursor.
  uint8_t action_;
  uint8_t current_patch_number_;
  uint8_t previous_patch_number_;
  uint8_t patch_buffer_[kSerializedPatchSize];
  // Buffer used to allow the user to undo the loading of a patch (similar to
  // the "compare" function on some synths).
  uint8_t patch_undo_buffer_[kSerializedPatchSize];
  
  // Output and Input handling for all the different category of pages.
  void DisplayEditSummaryPage();
  void DisplayEditDetailsPage();
  void HandleEditInput(uint8_t controller_index, uint16_t value);
  
  void DisplayLoadSavePage();
  void HandleLoadSaveInput(uint8_t controller_index, uint16_t value);
  void EnterLoadSaveMode();
  
  void DisplayStepSequencerPage();
  void HandleStepSequencerInput(uint8_t controller_index, uint16_t value);
  
  DISALLOW_COPY_AND_ASSIGN(Editor);
};

}  // namespace hardware_shruti

#endif  // HARDWARE_SHRUTI_EDITOR_H_