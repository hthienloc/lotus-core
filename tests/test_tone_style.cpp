#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

void test_tone_style_placement() {
    Engine engine;

    // Test NEW Style (Nucleus-centric: hoà) - Default
    engine.set_method(InputMethod::TELEX);
    engine.set_tone_style(ToneStyle::NEW);

    (void)engine.process_key('h', {});
    (void)engine.process_key('o', {});
    (void)engine.process_key('a', {});
    auto res4 = engine.process_key('f', {});

    // Expected: hoà
    std::u32string out_u32_new(res4.chars, res4.chars + res4.count);
    std::string out_new = unicode::to_utf8(out_u32_new);
    std::cout << "NEW (f): " << out_new << std::endl;
    // h + o + à = hoà. à is U+00E0.
    assert(out_new == "hoà");

    engine.reset();

    // Test OLD Style (Glide-centric: hòa)
    engine.set_tone_style(ToneStyle::OLD);
    engine.process_key('h', {});
    engine.process_key('o', {});
    engine.process_key('a', {});
    auto res_old = engine.process_key('f', {});

    // Expected: hòa
    std::u32string out_u32_old(res_old.chars, res_old.chars + res_old.count);
    std::string out_old = unicode::to_utf8(out_u32_old);
    std::cout << "OLD (f): " << out_old << std::endl;
    assert(out_old == "hòa");
}

void test_complex_diphthongs() {
    Engine engine;
    engine.set_method(InputMethod::TELEX);
    engine.set_tone_style(ToneStyle::NEW);

    auto type = [&](const std::string& keys) -> std::string {
        engine.reset();
        EngineResult res;
        for (char c : keys) {
            res = engine.process_key(c, {});
        }
        return unicode::to_utf8(std::u32string(res.chars, res.chars + res.count));
    };

    // triphthongs (iêu, uôi, ươi, ươu)
    std::cout << "DEBUG kieeur: " << type("kieeur") << std::endl;
    assert(type("kieeur") == "kiểu");

    std::cout << "DEBUG chuoois: " << type("chuoois") << std::endl;
    assert(type("chuoois") == "chuối");

    std::cout << "DEBUG ruowuj: " << type("ruowuj") << std::endl;
    assert(type("ruowuj") == "rượu");

    std::cout << "DEBUG huowu: " << type("huowu") << std::endl;
    assert(type("huowu") == "hươu");

    std::cout << "DEBUG nguowif: " << type("nguowif") << std::endl;
    assert(type("nguowif") == "người");
}
