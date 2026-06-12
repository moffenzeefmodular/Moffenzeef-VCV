#pragma once
// Auto-generated. Pointers into the embedded GMO bank rodata (see gmo_banks_data.S.in).
#include <cstdint>
#include "banks_manifest.hpp"

extern "C" {
extern const int8_t gmo_bank_blob_0[];
extern const int8_t gmo_bank_blob_1[];
extern const int8_t gmo_bank_blob_2[];
extern const int8_t gmo_bank_blob_3[];
extern const int8_t gmo_bank_blob_4[];
extern const int8_t gmo_bank_blob_5[];
extern const int8_t gmo_bank_blob_6[];
extern const int8_t gmo_bank_blob_7[];
extern const int8_t gmo_bank_blob_8[];
extern const int8_t gmo_bank_blob_9[];
extern const int8_t gmo_bank_blob_10[];
extern const int8_t gmo_bank_blob_11[];
extern const int8_t gmo_bank_blob_12[];
extern const int8_t gmo_bank_blob_13[];
extern const int8_t gmo_bank_blob_14[];
extern const int8_t gmo_bank_blob_15[];
extern const int8_t gmo_bank_blob_16[];
extern const int8_t gmo_bank_blob_17[];
extern const int8_t gmo_bank_blob_18[];
extern const int8_t gmo_bank_blob_19[];
extern const int8_t gmo_bank_blob_20[];
extern const int8_t gmo_bank_blob_21[];
extern const int8_t gmo_bank_blob_22[];
extern const int8_t gmo_bank_blob_23[];
extern const int8_t gmo_bank_blob_24[];
extern const int8_t gmo_bank_blob_25[];
extern const int8_t gmo_bank_blob_26[];
extern const int8_t gmo_bank_blob_27[];
extern const int8_t gmo_bank_blob_28[];
extern const int8_t gmo_bank_blob_29[];
extern const int8_t gmo_bank_blob_30[];
extern const int8_t gmo_bank_blob_31[];
extern const int8_t gmo_bank_blob_32[];
extern const int8_t gmo_bank_blob_33[];
extern const int8_t gmo_bank_blob_34[];
extern const int8_t gmo_bank_blob_35[];
extern const int8_t gmo_bank_blob_36[];
extern const int8_t gmo_bank_blob_37[];
extern const int8_t gmo_bank_blob_38[];
extern const int8_t gmo_bank_blob_39[];
extern const int8_t gmo_bank_blob_40[];
extern const int8_t gmo_bank_blob_41[];
extern const int8_t gmo_bank_blob_42[];
}

// In GMO_BANKS[] order. "+ 1" skips the leading numWavetables byte in each .bin,
// so each pointer addresses the first wavetable sample (WAVETABLE_SIZE int8 per table).
static const int8_t* const GMO_BANK_DATA[GMO_NUM_BANKS] = {
	gmo_bank_blob_0 + 1,
	gmo_bank_blob_1 + 1,
	gmo_bank_blob_2 + 1,
	gmo_bank_blob_3 + 1,
	gmo_bank_blob_4 + 1,
	gmo_bank_blob_5 + 1,
	gmo_bank_blob_6 + 1,
	gmo_bank_blob_7 + 1,
	gmo_bank_blob_8 + 1,
	gmo_bank_blob_9 + 1,
	gmo_bank_blob_10 + 1,
	gmo_bank_blob_11 + 1,
	gmo_bank_blob_12 + 1,
	gmo_bank_blob_13 + 1,
	gmo_bank_blob_14 + 1,
	gmo_bank_blob_15 + 1,
	gmo_bank_blob_16 + 1,
	gmo_bank_blob_17 + 1,
	gmo_bank_blob_18 + 1,
	gmo_bank_blob_19 + 1,
	gmo_bank_blob_20 + 1,
	gmo_bank_blob_21 + 1,
	gmo_bank_blob_22 + 1,
	gmo_bank_blob_23 + 1,
	gmo_bank_blob_24 + 1,
	gmo_bank_blob_25 + 1,
	gmo_bank_blob_26 + 1,
	gmo_bank_blob_27 + 1,
	gmo_bank_blob_28 + 1,
	gmo_bank_blob_29 + 1,
	gmo_bank_blob_30 + 1,
	gmo_bank_blob_31 + 1,
	gmo_bank_blob_32 + 1,
	gmo_bank_blob_33 + 1,
	gmo_bank_blob_34 + 1,
	gmo_bank_blob_35 + 1,
	gmo_bank_blob_36 + 1,
	gmo_bank_blob_37 + 1,
	gmo_bank_blob_38 + 1,
	gmo_bank_blob_39 + 1,
	gmo_bank_blob_40 + 1,
	gmo_bank_blob_41 + 1,
	gmo_bank_blob_42 + 1,
};
