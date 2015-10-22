// Copyright (c) 2015 The Khronos Group Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and/or associated documentation files (the
// "Materials"), to deal in the Materials without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Materials, and to
// permit persons to whom the Materials are furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Materials.
//
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
//
// THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

// Assembler tests for instructions in the "Control Flow" section of the
// SPIR-V spec.

#include "UnitSPIRV.h"

#include "gmock/gmock.h"
#include "TestFixture.h"

namespace {

using spvtest::EnumCase;
using spvtest::MakeInstruction;
using spvtest::Concatenate;
using spvtest::TextToBinaryTest;
using ::testing::Eq;

// Test OpSelectionMerge

using OpSelectionMergeTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<EnumCase<spv::SelectionControlMask>>>;

TEST_P(OpSelectionMergeTest, AnySingleSelectionControlMask) {
  const std::string input = "OpSelectionMerge %1 " + GetParam().name();
  EXPECT_THAT(
      CompiledInstructions(input),
      Eq(MakeInstruction(spv::OpSelectionMerge, {1, GetParam().value()})));
}

// clang-format off
#define CASE(VALUE,NAME) { spv::SelectionControl##VALUE, NAME}
INSTANTIATE_TEST_CASE_P(TextToBinarySelectionMerge, OpSelectionMergeTest,
                        ::testing::ValuesIn(std::vector<EnumCase<spv::SelectionControlMask>>{
                            CASE(MaskNone, "None"),
                            CASE(FlattenMask, "Flatten"),
                            CASE(DontFlattenMask, "DontFlatten"),
                        }));
#undef CASE
// clang-format on

TEST_F(OpSelectionMergeTest, CombinedSelectionControlMask) {
  const std::string input = "OpSelectionMerge %1 Flatten|DontFlatten";
  const uint32_t expected_mask =
      spv::SelectionControlFlattenMask | spv::SelectionControlDontFlattenMask;
  EXPECT_THAT(CompiledInstructions(input),
              Eq(MakeInstruction(spv::OpSelectionMerge, {1, expected_mask})));
}

TEST_F(OpSelectionMergeTest, WrongSelectionControl) {
  // Case sensitive: "flatten" != "Flatten" and thus wrong.
  EXPECT_THAT(CompileFailure("OpSelectionMerge %1 flatten|DontFlatten"),
              Eq("Invalid selection control 'flatten|DontFlatten'."));
}

// Test OpLoopMerge

using OpLoopMergeTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<EnumCase<spv::LoopControlMask>>>;

TEST_P(OpLoopMergeTest, AnySingleLoopControlMask) {
  const std::string input = "OpLoopMerge %merge %continue " + GetParam().name();
  EXPECT_THAT(
      CompiledInstructions(input),
      Eq(MakeInstruction(spv::OpLoopMerge, {1, 2, GetParam().value()})));
}

// clang-format off
#define CASE(VALUE,NAME) { spv::LoopControl##VALUE, NAME}
INSTANTIATE_TEST_CASE_P(TextToBinaryLoopMerge, OpLoopMergeTest,
                        ::testing::ValuesIn(std::vector<EnumCase<spv::LoopControlMask>>{
                            CASE(MaskNone, "None"),
                            CASE(UnrollMask, "Unroll"),
                            CASE(DontUnrollMask, "DontUnroll"),
                        }));
#undef CASE
// clang-format on

TEST_F(OpLoopMergeTest, CombinedLoopControlMask) {
  const std::string input = "OpLoopMerge %merge %continue Unroll|DontUnroll";
  const uint32_t expected_mask =
      spv::LoopControlUnrollMask | spv::LoopControlDontUnrollMask;
  EXPECT_THAT(CompiledInstructions(input),
              Eq(MakeInstruction(spv::OpLoopMerge, {1, 2, expected_mask})));
}

TEST_F(OpLoopMergeTest, WrongLoopControl) {
  EXPECT_THAT(CompileFailure("OpLoopMerge %m %c none"),
              Eq("Invalid loop control 'none'."));
}

// Test OpSwitch

TEST_F(TextToBinaryTest, SwitchGoodZeroTargets) {
  EXPECT_THAT(CompiledInstructions("OpSwitch %selector %default"),
              Eq(MakeInstruction(spv::OpSwitch, {1, 2})));
}

TEST_F(TextToBinaryTest, SwitchGoodOneTarget) {
  EXPECT_THAT(CompiledInstructions("%1 = OpTypeInt 32 0\n"
                                   "%2 = OpConstant %1 52\n"
                                   "OpSwitch %2 %default 12 %target0"),
              Eq(Concatenate({MakeInstruction(spv::OpTypeInt, {1, 32, 0}),
                              MakeInstruction(spv::OpConstant, {1, 2, 52}),
                              MakeInstruction(spv::OpSwitch, {2, 3, 12, 4})})));
}

TEST_F(TextToBinaryTest, SwitchGoodTwoTargets) {
  EXPECT_THAT(
      CompiledInstructions("%1 = OpTypeInt 32 0\n"
                           "%2 = OpConstant %1 52\n"
                           "OpSwitch %2 %default 12 %target0 42 %target1"),
      Eq(Concatenate({
          MakeInstruction(spv::OpTypeInt, {1, 32, 0}),
          MakeInstruction(spv::OpConstant, {1, 2, 52}),
          MakeInstruction(spv::OpSwitch, {2, 3, 12, 4, 42, 5}),
      })));
}

TEST_F(TextToBinaryTest, SwitchBadMissingSelector) {
  EXPECT_THAT(CompileFailure("OpSwitch"),
              Eq("Expected operand, found end of stream."));
}

TEST_F(TextToBinaryTest, SwitchBadInvalidSelector) {
  EXPECT_THAT(CompileFailure("OpSwitch 12"),
              Eq("Expected id to start with %."));
}

TEST_F(TextToBinaryTest, SwitchBadMissingDefault) {
  EXPECT_THAT(CompileFailure("OpSwitch %selector"),
              Eq("Expected operand, found end of stream."));
}

TEST_F(TextToBinaryTest, SwitchBadInvalidDefault) {
  EXPECT_THAT(CompileFailure("OpSwitch %selector 12"),
              Eq("Expected id to start with %."));
}

TEST_F(TextToBinaryTest, SwitchBadInvalidLiteralDefaultFormat) {
  // The assembler recognizes "OpSwitch %selector %default" as a complete
  // instruction.  Then it tries to parse "%abc" as the start of an
  // assignment form instruction, but can't since it hits the end
  // of stream.
  const auto input = R"(%i32 = OpTypeInt 32 0
                        %selector = OpConstant %i32 42
                        OpSwitch %selector %default %abc)";
  EXPECT_THAT(CompileFailure(input), Eq("Expected '=', found end of stream."));
}

TEST_F(TextToBinaryTest, SwitchBadInvalidLiteralCanonicalFormat) {
  const auto input = R"(OpTypeInt %i32 32 0
                        OpConstant %i32 %selector 42
                  OpSwitch %selector %default %abc)";
  EXPECT_THAT(CompileFailure(input, SPV_ASSEMBLY_SYNTAX_FORMAT_CANONICAL),
              Eq("Expected <opcode> at the beginning of an instruction, found "
                 "'%abc'."));
}

TEST_F(TextToBinaryTest, SwitchBadMissingTarget) {
  EXPECT_THAT(CompileFailure("%1 = OpTypeInt 32 0\n"
                             "%2 = OpConstant %1 52\n"
                             "OpSwitch %2 %default 12"),
              Eq("Expected operand, found end of stream."));
}

// A test case for an OpSwitch.
// It is also parameterized to test encodings OpConstant
// integer literals.  This can capture both single and multi-word
// integer literal tests.
struct SwitchTestCase {
  std::string constant_type_args;
  std::string constant_value_arg;
  std::string case_value_arg;
  std::vector<uint32_t> expected_instructions;
};

using OpSwitchValidTest =
    spvtest::TextToBinaryTestBase<::testing::TestWithParam<SwitchTestCase>>;

// Tests the encoding of OpConstant literal values, and also
// the literal integer cases in an OpSwitch.  This can
// test both single and multi-word integer literal encodings.
TEST_P(OpSwitchValidTest, ValidTypes) {
  const std::string input = "%1 = OpTypeInt " + GetParam().constant_type_args +
                            "\n"
                            "%2 = OpConstant %1 " +
                            GetParam().constant_value_arg +
                            "\n"
                            "OpSwitch %2 %default " +
                            GetParam().case_value_arg + " %4\n";
  std::vector<uint32_t> instructions;
  EXPECT_THAT(CompiledInstructions(input),
              Eq(GetParam().expected_instructions));
}

// Constructs a SwitchTestCase from the given integer_width, signedness,
// constant value string, and expected encoded constant.
SwitchTestCase MakeSwitchTestCase(uint32_t integer_width,
                                  uint32_t integer_signedness,
                                  std::string constant_str,
                                  std::vector<uint32_t> encoded_constant,
                                  std::string case_value_str,
                                  std::vector<uint32_t> encoded_case_value) {
  std::stringstream ss;
  ss << integer_width << " " << integer_signedness;
  return SwitchTestCase{
      ss.str(),
      constant_str,
      case_value_str,
      {Concatenate(
          {MakeInstruction(spv::OpTypeInt,
                           {1, integer_width, integer_signedness}),
           MakeInstruction(spv::OpConstant,
                           Concatenate({{1, 2}, encoded_constant})),
           MakeInstruction(spv::OpSwitch,
                           Concatenate({{2, 3}, encoded_case_value, {4}}))})}};
}

INSTANTIATE_TEST_CASE_P(
    TextToBinaryOpSwitchValid1Word, OpSwitchValidTest,
    ::testing::ValuesIn(std::vector<SwitchTestCase>({
        MakeSwitchTestCase(32, 0, "42", {42}, "100", {100}),
        MakeSwitchTestCase(32, 1, "-1", {0xffffffff}, "100", {100}),
        // SPIR-V 1.0 Rev 1 clarified that for an integer narrower than 32-bits,
        // its bits will appear in the lower order bits of the 32-bit word, and
        // a signed integer is sign-extended.
        MakeSwitchTestCase(7, 0, "127", {127}, "100", {100}),
        MakeSwitchTestCase(14, 0, "99", {99}, "100", {100}),
        MakeSwitchTestCase(16, 0, "65535", {65535}, "100", {100}),
        MakeSwitchTestCase(16, 1, "101", {101}, "100", {100}),
        // Demonstrate sign extension
        MakeSwitchTestCase(16, 1, "-2", {0xfffffffe}, "100", {100}),
        // Hex cases
        MakeSwitchTestCase(16, 1, "0x7ffe", {0x7ffe}, "0x1234", {0x1234}),
        MakeSwitchTestCase(16, 1, "0x8000", {0xffff8000}, "0x8100",
                           {0xffff8100}),
        MakeSwitchTestCase(16, 0, "0x8000", {0x00008000}, "0x8100", {0x8100}),
    })));

// NB: The words LOW ORDER bits show up first.
INSTANTIATE_TEST_CASE_P(
    TextToBinaryOpSwitchValid2Words, OpSwitchValidTest,
    ::testing::ValuesIn(std::vector<SwitchTestCase>({
        MakeSwitchTestCase(33, 0, "101", {101, 0}, "500", {500, 0}),
        MakeSwitchTestCase(48, 1, "-1", {0xffffffff, 0xffffffff}, "900",
                           {900, 0}),
        MakeSwitchTestCase(64, 1, "-2", {0xfffffffe, 0xffffffff}, "-5",
                           {0xfffffffb, uint32_t(-1)}),
        // Hex cases
        MakeSwitchTestCase(48, 1, "0x7fffffffffff", {0xffffffff, 0x00007fff},
                           "100", {100, 0}),
        MakeSwitchTestCase(48, 1, "0x800000000000", {0x00000000, 0xffff8000},
                           "0x800000000000", {0x00000000, 0xffff8000}),
        MakeSwitchTestCase(48, 0, "0x800000000000", {0x00000000, 0x00008000},
                           "0x800000000000", {0x00000000, 0x00008000}),
        MakeSwitchTestCase(63, 0, "0x500000000", {0, 5}, "12", {12, 0}),
        MakeSwitchTestCase(64, 0, "0x600000000", {0, 6}, "12", {12, 0}),
        MakeSwitchTestCase(64, 1, "0x700000123", {0x123, 7}, "12", {12, 0}),
    })));

using RoundTripTest =
    spvtest::TextToBinaryTestBase<::testing::TestWithParam<std::string>>;

TEST_P(RoundTripTest, Sample) {
  EXPECT_THAT(EncodeAndDecodeSuccessfully(GetParam()), Eq(GetParam()));
}

// TODO(dneto): Enable this test.
INSTANTIATE_TEST_CASE_P(
    DISABLED_OpSwitchRoundTripUnsignedIntegers, RoundTripTest,
    ::testing::ValuesIn(std::vector<std::string>({
        // Unsigned 16-bit.
        "%1 = OpTypeInt 16 0\n%2 = OpConstant %1 65535\nOpSwitch %2 %3\n",
        // Unsigned 32-bit, three non-default cases.
        "%1 = OpTypeInt 32 0\n%2 = OpConstant %1 123456\n"
        "OpSwitch %2 %3 100 %4 102 %5 1000000 %6\n",
        // Unsigned 48-bit, three non-default cases.
        "%1 = OpTypeInt 48 0\n%2 = OpConstant %1 5000000000\n"
        "OpSwitch %2 %3 100 %4 102 %5 6000000000 %6\n",
        // Unsigned 64-bit, three non-default cases.
        "%1 = OpTypeInt 64 0\n%2 = OpConstant %1 9223372036854775807\n"
        "OpSwitch %2 %3 100 %4 102 %5 9000000000000000000 %6\n",
    })));

INSTANTIATE_TEST_CASE_P(
    DISABLED_OpSwitchRoundTripSignedIntegers, RoundTripTest,
    ::testing::ValuesIn(std::vector<std::string>{
        // Signed 16-bit, with two non-default cases
        "%1 = OpTypeInt 16 1\n%2 = OpConstant %1 32767\n"
        "OpSwitch %2 %3 99 %4 -102 %5\n",
        "%1 = OpTypeInt 16 1\n%2 = OpConstant %1 -32768\n"
        "OpSwitch %2 %3 99 %4 -102 %5\n",
        // Signed 32-bit, two non-default cases.
        "%1 = OpTypeInt 32 1\n%2 = OpConstant %1 -123456\n"
        "OpSwitch %2 %3 100 %4 -102\n",
        "%1 = OpTypeInt 32 1\n%2 = OpConstant %1 123456\n"
        "OpSwitch %2 %3 100 %4 -102\n",
        // Signed 48-bit, three non-default cases.
        "%1 = OpTypeInt 48 1\n%2 = OpConstant %1 5000000000\n"
        "OpSwitch %2 %3 100 %4 -7000000000 %5 6000000000 %6\n",
        "%1 = OpTypeInt 48 1\n%2 = OpConstant %1 -5000000000\n"
        "OpSwitch %2 %3 100 %4 -7000000000 %5 6000000000 %6\n",
        // Signed 64-bit, three non-default cases.
        "%1 = OpTypeInt 64 1\n%2 = OpConstant %1 9223372036854775807\n"
        "OpSwitch %2 %3 100 %4 7000000000 %5 -1000000000000000000 %6\n",
        "%1 = OpTypeInt 64 1\n%2 = OpConstant %1 -9223372036854775808\n"
        "OpSwitch %2 %3 100 %4 7000000000 %5 -1000000000000000000 %6\n",
    }));

using OpSwitchInvalidTypeTestCase =
    spvtest::TextToBinaryTestBase<::testing::TestWithParam<std::string>>;

TEST_P(OpSwitchInvalidTypeTestCase, InvalidTypes) {
  const std::string input =
      "%1 = " + GetParam() +
      "\n"
      "%3 = OpCopyObject %1 %2\n"  // We only care the type of the expression
      "%4 = OpSwitch %3 %default 32 %c\n";
  EXPECT_THAT(CompileFailure(input),
              Eq("The selector operand for OpSwitch must be the result of an "
                 "instruction that generates an integer scalar"));
}

// clang-format off
INSTANTIATE_TEST_CASE_P(
    TextToBinaryOpSwitchInvalidTests, OpSwitchInvalidTypeTestCase,
    ::testing::ValuesIn(std::vector<std::string>{
      {"OpTypeVoid",
       "OpTypeBool",
       "OpTypeFloat 32",
       "OpTypeVector %a 32",
       "OpTypeMatrix %a 32",
       "OpTypeImage %a 1D 0 0 0 0 Unknown",
       "OpTypeSampler",
       "OpTypeSampledImage %a",
       "OpTypeArray %a %b",
       "OpTypeRuntimeArray %a",
       "OpTypeStruct %a",
       "OpTypeOpaque \"Foo\"",
       "OpTypePointer UniformConstant %a",
       "OpTypeFunction %a %b",
       "OpTypeEvent",
       "OpTypeDeviceEvent",
       "OpTypeReserveId",
       "OpTypeQueue",
       "OpTypePipe ReadOnly",
       "OpTypeForwardPointer %a UniformConstant",
           // At least one thing that isn't a type at all
       "OpNot %a %b"
      },
    }));
// clang-format on

// TODO(dneto): OpPhi
// TODO(dneto): OpLoopMerge
// TODO(dneto): OpLabel
// TODO(dneto): OpBranch
// TODO(dneto): OpSwitch
// TODO(dneto): OpKill
// TODO(dneto): OpReturn
// TODO(dneto): OpReturnValue
// TODO(dneto): OpUnreachable
// TODO(dneto): OpLifetimeStart
// TODO(dneto): OpLifetimeStop

}  // anonymous namespace
