/**
 * Native unit tests for the function map and vehicle model presets.
 *
 * Tests run on the host using PlatformIO's Unity framework.
 * No hardware is required.
 */

#include <FunctionMap.h>
#include <Protocol.h>
#include <cstdint>
#include <cstring>
#include <unity.h>

using namespace odh;

/* ── FunctionMapEntry size ───────────────────────────────────────────────── */

void test_function_map_entry_size(void) {
    TEST_ASSERT_EQUAL(2, static_cast<int>(sizeof(FunctionMapEntry)));
}

void test_telemetry_packet_has_model_type(void) {
    TelemetryPacket pkt{};
    pkt.model_type = static_cast<uint8_t>(ModelType::Excavator);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ModelType::Excavator), pkt.model_type);
    TEST_ASSERT_LESS_OR_EQUAL(250, static_cast<int>(sizeof(TelemetryPacket)));
}

/* ── Model presets ───────────────────────────────────────────────────────── */

void test_generic_model_has_drive_and_steering(void) {
    auto m = defaultFunctionMap(ModelType::Generic);
    TEST_ASSERT_GREATER_OR_EQUAL(2, m.count);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Drive), m[0].function);
    TEST_ASSERT_EQUAL_UINT8(0, m[0].channel);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Steering), m[1].function);
    TEST_ASSERT_EQUAL_UINT8(1, m[1].channel);
}

void test_dump_truck_has_dump_bed(void) {
    auto m = defaultFunctionMap(ModelType::DumpTruck);
    TEST_ASSERT_EQUAL_UINT8(3, m.count);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::DumpBed), m[2].function);
}

void test_excavator_has_six_functions(void) {
    auto m = defaultFunctionMap(ModelType::Excavator);
    TEST_ASSERT_EQUAL_UINT8(6, m.count);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::TrackL), m[0].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::TrackR), m[1].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::BoomUD), m[2].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::BoomLR), m[3].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Bucket), m[4].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Swing), m[5].function);
}

void test_tractor_has_four_functions(void) {
    auto m = defaultFunctionMap(ModelType::Tractor);
    TEST_ASSERT_EQUAL_UINT8(4, m.count);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Pto), m[2].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Hitch), m[3].function);
}

void test_crane_has_five_functions(void) {
    auto m = defaultFunctionMap(ModelType::Crane);
    TEST_ASSERT_EQUAL_UINT8(5, m.count);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Winch), m[3].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Swing), m[4].function);
}

void test_unknown_model_falls_back_to_generic(void) {
    auto m = defaultFunctionMap(static_cast<uint8_t>(0xFE));
    TEST_ASSERT_GREATER_OR_EQUAL(2, m.count);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Drive), m[0].function);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Steering), m[1].function);
}

/* ── Mapping lookup ──────────────────────────────────────────────────────── */

void test_function_to_channel_found(void) {
    auto m  = defaultFunctionMap(ModelType::Excavator);
    auto ch = functionToChannel(m, Function::Bucket);
    TEST_ASSERT_TRUE(ch.has_value());
    TEST_ASSERT_EQUAL_UINT8(4, ch.value());
}

void test_function_to_channel_not_found(void) {
    auto m  = defaultFunctionMap(ModelType::DumpTruck);
    auto ch = functionToChannel(m, Function::Winch);
    TEST_ASSERT_FALSE(ch.has_value());
}

void test_channel_to_function_found(void) {
    auto m    = defaultFunctionMap(ModelType::Tractor);
    auto func = channelToFunction(m, 2);
    TEST_ASSERT_TRUE(func.has_value());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Function::Pto),
                            static_cast<uint8_t>(func.value()));
}

void test_channel_to_function_not_found(void) {
    auto m    = defaultFunctionMap(ModelType::DumpTruck);
    auto func = channelToFunction(m, 7);
    TEST_ASSERT_FALSE(func.has_value());
}

/* ── Function and model name labels ──────────────────────────────────────── */

void test_function_name_drive(void) {
    auto name = functionName(Function::Drive);
    // Compare via c_str for string_view or const char*
    TEST_ASSERT_TRUE(name == NameView("Drive"));
}

void test_function_name_unknown(void) {
    auto name = functionName(Function::None);
    TEST_ASSERT_TRUE(name == NameView("Unknown"));
}

void test_model_name_excavator(void) {
    auto name = modelName(ModelType::Excavator);
    TEST_ASSERT_TRUE(name == NameView("Excavator"));
}

void test_model_name_unknown(void) {
    auto name = modelName(static_cast<uint8_t>(0xFE));
    TEST_ASSERT_TRUE(name == NameView("Unknown"));
}

/* ── Channel assignments are unique per model ────────────────────────────── */

void test_channel_assignments_unique(void) {
    for (uint8_t model = 0; model < static_cast<uint8_t>(ModelType::Count); model++) {
        auto m = defaultFunctionMap(model);
        for (uint8_t i = 0; i < m.count; i++) {
            for (uint8_t j = i + 1; j < m.count; j++) {
                TEST_ASSERT_NOT_EQUAL(m[i].channel, m[j].channel);
            }
        }
    }
}
