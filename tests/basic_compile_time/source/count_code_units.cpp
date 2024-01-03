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

#include <ztd/text/tests/basic_unicode_strings.hpp>

#include <ztd/text/count_as_decoded.hpp>
#include <ztd/text/encoding.hpp>
#include <ztd/text/is_unicode_encoding.hpp>

inline namespace ztd_text_tests_basic_compile_time_count_as_decoded {

	template <typename T>
	static void delayed() {
		static_assert(ztd::text::count_as_decoded(ztd::tests::u8_basic_source_character_set).count
		     == ztd::tests::u32_basic_source_character_set.size());
		static_assert(ztd::text::count_as_decoded(ztd::tests::u16_basic_source_character_set).count
		     == ztd::tests::u32_basic_source_character_set.size());
		static_assert(ztd::text::count_as_decoded(ztd::tests::u32_basic_source_character_set).count
		     == ztd::tests::u32_basic_source_character_set.size());

		// Basic Source Character Set, explicit encoding
		static_assert(ztd::text::count_as_decoded(ztd::tests::basic_source_character_set, ztd::text::literal).count
		     == ztd::tests::u32_basic_source_character_set.size());
		static_assert(
		     ztd::text::count_as_decoded(ztd::tests::w_basic_source_character_set, ztd::text::wide_literal).count
		     == ztd::tests::u32_basic_source_character_set.size());
		static_assert(ztd::text::count_as_decoded(ztd::tests::u8_basic_source_character_set, ztd::text::utf8).count
		     == ztd::tests::u32_basic_source_character_set.size());
		static_assert(ztd::text::count_as_decoded(ztd::tests::u16_basic_source_character_set, ztd::text::utf16).count
		     == ztd::tests::u32_basic_source_character_set.size());
		static_assert(ztd::text::count_as_decoded(ztd::tests::u32_basic_source_character_set, ztd::text::utf32).count
		     == ztd::tests::u32_basic_source_character_set.size());

		static_assert(ztd::text::count_as_decoded(ztd::tests::u8_unicode_sequence_truth_native_endian).count
		     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());
		static_assert(ztd::text::count_as_decoded(ztd::tests::u16_unicode_sequence_truth_native_endian).count
		     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());
		static_assert(ztd::text::count_as_decoded(ztd::tests::u32_unicode_sequence_truth_native_endian).count
		     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());

		// Larger unicode sequences, explicit encoding
		if constexpr (ztd::always_true_v<T> && ztd::text::is_unicode_encoding_v<ztd::text::literal_t>) {
			static_assert(ztd::text::count_as_decoded(ztd::tests::unicode_sequence_truth_native_endian,
			                   ztd::text::literal, ztd::text::replacement_handler)
			                   .count
			     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());
		}
		if constexpr (ztd::always_true_v<T> && ztd::text::is_unicode_encoding_v<ztd::text::wide_literal_t>) {
			static_assert(ztd::text::count_as_decoded(ztd::tests::w_unicode_sequence_truth_native_endian,
			                   ztd::text::wide_literal, ztd::text::replacement_handler)
			                   .count
			     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());
		}
		static_assert(
		     ztd::text::count_as_decoded(ztd::tests::u8_unicode_sequence_truth_native_endian, ztd::text::utf8).count
		     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());
		static_assert(
		     ztd::text::count_as_decoded(ztd::tests::u16_unicode_sequence_truth_native_endian, ztd::text::utf16).count
		     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());
		static_assert(
		     ztd::text::count_as_decoded(ztd::tests::u32_unicode_sequence_truth_native_endian, ztd::text::utf32).count
		     == ztd::tests::u32_unicode_sequence_truth_native_endian.size());
	}

	void instantiate() {
		delayed<void>();
	}

} // namespace ztd_text_tests_basic_compile_time_count_as_decoded
