#include "lotus_engine/capi.h"

#include <cassert>
#include <iostream>

void test_capi_integration() {
    lotus_engine_t* engine = lotus_engine_create();
    assert(engine != nullptr);

    lotus_modifiers_t mods = {false, false};

    // 1. Basic Telex Input (hòa vs hoà)
    // Style: OLD -> "hòa" (Mark on 'o')
    lotus_engine_set_tone_style(engine, LOTUS_TONE_OLD);
    lotus_engine_process_key(engine, 'h', mods);
    lotus_engine_process_key(engine, 'o', mods);
    lotus_engine_process_key(engine, 'a', mods);
    lotus_result_t res = lotus_engine_process_key(engine, 'f', mods);

    // Characters: h (U+0068), ò (U+00F2), a (U+0061)
    assert(res.count == 3);
    assert(res.chars[0] == 0x0068);
    assert(res.chars[1] == 0x00F2);
    assert(res.chars[2] == 0x0061);

    // Resetting engine
    lotus_engine_reset(engine);

    // Style: NEW -> "hoà" (Mark on 'a')
    lotus_engine_set_tone_style(engine, LOTUS_TONE_NEW);
    lotus_engine_process_key(engine, 'h', mods);
    lotus_engine_process_key(engine, 'o', mods);
    lotus_engine_process_key(engine, 'a', mods);
    res = lotus_engine_process_key(engine, 'f', mods);

    // Characters: h (U+0068), o (U+006F), à (U+00E0)
    assert(res.count == 3);
    assert(res.chars[0] == 0x0068);
    assert(res.chars[1] == 0x006F);
    assert(res.chars[2] == 0x00E0);

    // Resetting engine
    lotus_engine_reset(engine);

    // 2. VNI Input
    lotus_engine_set_method(engine, LOTUS_METHOD_VNI);
    lotus_engine_process_key(engine, 'v', mods);
    lotus_engine_process_key(engine, 'i', mods);
    lotus_engine_process_key(engine, 'e', mods);
    lotus_engine_process_key(engine, '6', mods);  // 6 -> ê
    lotus_engine_process_key(engine, 't', mods);
    res = lotus_engine_process_key(engine, '5', mods);  // 5 -> nặng (ệ)

    // việt -> v (U+0076), i (U+0069), ệ (U+1EC7), t (U+0074)
    assert(res.count == 4);
    assert(res.chars[0] == 0x0076);
    assert(res.chars[1] == 0x0069);
    assert(res.chars[2] == 0x1EC7);
    assert(res.chars[3] == 0x0074);

    lotus_engine_destroy(engine);
    std::cout << "test_capi_integration PASSED" << std::endl;
}
