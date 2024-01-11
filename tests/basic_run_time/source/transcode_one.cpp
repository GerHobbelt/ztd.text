// =============================================================================
//
// ztd.text
// Copyright © JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
// Contact: opensource@soasis.org
//
// Commercial License Usage
// Licensees holding valid commercial ztd.text licenses may use this file in
// accordance with the commercial license agreement provided with the
// Software or, alternatively, in accordance with the terms contained in
// a written agreement between you and Shepherd's Oasis, LLC.
// For licensing terms and conditions see your agreement. For
// further information contact opensource@soasis.org.
//
// Apache License Version 2 Usage
// Alternatively, this file may be used under the terms of Apache License
// Version 2.0 (the "License") for non-commercial use; you may not use this
// file except in compliance with the License. You may obtain a copy of the
// License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ============================================================================ //

#include <ztd/text/transcode_one.hpp>

#include <ztd/text/encoding.hpp>

#include <catch2/catch_all.hpp>

#include <ztd/text/tests/basic_unicode_strings.hpp>

inline namespace ztd_text_tests_basic_run_time_transcode_one {
	template <typename FromEncoding, typename ToEncoding, typename Source, typename Expected>
	void check_transcode_one(
	     FromEncoding& from_encoding, ToEncoding& to_encoding, Source& source, Expected& expected) {
		using OutChar                    = ztd::text::code_unit_t<ToEncoding>;
		constexpr std::size_t MaxOutSize = ztd::text::max_transcode_code_units_v<FromEncoding, ToEncoding>;
		using Output                     = ztd::static_basic_string<OutChar, MaxOutSize>;
		auto source_it                   = source.cbegin();
		auto source_last                 = source.cend();
		auto expected_it                 = expected.cbegin();
		auto expected_last               = expected.cend();
		for (; expected_it != expected_last && source_it != source_last;) {
			const auto result0 = ztd::text::transcode_one_to(ztd::ranges::make_subrange(source_it, source_last),
			     from_encoding, to_encoding, ztd::text::replacement_handler);
			const Output& result0_output = result0.output;
			const bool result0_is_equal_to_expected
			     = std::equal(result0_output.cbegin(), result0_output.cend(), expected_it);
			REQUIRE(result0.error_code == ztd::text::encoding_error::ok);
			REQUIRE_FALSE(result0.errors_were_handled());
			REQUIRE(result0_is_equal_to_expected);
			// make sure that the transcode_one branch of functions is also tested
			const Output independent_result0_output
			     = ztd::text::transcode_one(ztd::ranges::make_subrange(source_it, source_last), from_encoding,
			          to_encoding, ztd::text::replacement_handler);
			REQUIRE(result0_output == independent_result0_output);
			expected_it += result0_output.size();
			source_it = std::move(result0.input.begin());
		}
		REQUIRE(source_it == source_last);
		REQUIRE(expected_it == expected_last);
	}
} // namespace ztd_text_tests_basic_run_time_transcode_one

TEST_CASE(
     "transcode_one can roundtrip through data one indivisible unit at a time", "[text][transcode_one][roundtrip]") {
	SECTION("execution") {
		check_transcode_one(ztd::text::execution, ztd::text::execution, ztd::tests::basic_source_character_set,
		     ztd::tests::basic_source_character_set);

		if (ztd::text::contains_unicode_encoding(ztd::text::execution)) {
			check_transcode_one(ztd::text::execution, ztd::text::execution,
			     ztd::tests::unicode_sequence_truth_native_endian, ztd::tests::unicode_sequence_truth_native_endian);
		}
	}
	SECTION("wide_execution") {
		check_transcode_one(ztd::text::wide_execution, ztd::text::wide_execution,
		     ztd::tests::w_basic_source_character_set, ztd::tests::w_basic_source_character_set);
		if (ztd::text::contains_unicode_encoding(ztd::text::wide_execution)) {
			check_transcode_one(ztd::text::wide_execution, ztd::text::wide_execution,
			     ztd::tests::w_unicode_sequence_truth_native_endian,
			     ztd::tests::w_unicode_sequence_truth_native_endian);
		}
	}
	SECTION("literal") {
		check_transcode_one(ztd::text::literal, ztd::text::literal, ztd::tests::basic_source_character_set,
		     ztd::tests::basic_source_character_set);
		if (ztd::text::contains_unicode_encoding(ztd::text::literal)) {
			check_transcode_one(ztd::text::literal, ztd::text::literal,
			     ztd::tests::unicode_sequence_truth_native_endian, ztd::tests::unicode_sequence_truth_native_endian);
		}
	}
	SECTION("wide_literal") {
		check_transcode_one(ztd::text::wide_literal, ztd::text::wide_literal,
		     ztd::tests::w_basic_source_character_set, ztd::tests::w_basic_source_character_set);
		if (ztd::text::contains_unicode_encoding(ztd::text::wide_literal)) {
			check_transcode_one(ztd::text::wide_literal, ztd::text::wide_literal,
			     ztd::tests::w_unicode_sequence_truth_native_endian,
			     ztd::tests::w_unicode_sequence_truth_native_endian);
		}
	}
	SECTION("utf8") {
		check_transcode_one(ztd::text::utf8, ztd::text::utf8, ztd::tests::u8_basic_source_character_set,
		     ztd::tests::u8_basic_source_character_set);
		check_transcode_one(ztd::text::utf8, ztd::text::utf8, ztd::tests::u8_unicode_sequence_truth_native_endian,
		     ztd::tests::u8_unicode_sequence_truth_native_endian);
	}
	SECTION("utf16") {
		check_transcode_one(ztd::text::utf16, ztd::text::utf16, ztd::tests::u16_basic_source_character_set,
		     ztd::tests::u16_basic_source_character_set);
		check_transcode_one(ztd::text::utf16, ztd::text::utf16, ztd::tests::u16_unicode_sequence_truth_native_endian,
		     ztd::tests::u16_unicode_sequence_truth_native_endian);
	}
	SECTION("utf32") {
		check_transcode_one(ztd::text::utf32, ztd::text::utf32, ztd::tests::u32_basic_source_character_set,
		     ztd::tests::u32_basic_source_character_set);
		check_transcode_one(ztd::text::utf32, ztd::text::utf32, ztd::tests::u32_unicode_sequence_truth_native_endian,
		     ztd::tests::u32_unicode_sequence_truth_native_endian);
	}
}
