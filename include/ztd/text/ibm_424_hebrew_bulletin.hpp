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

#pragma once

#ifndef ZTD_TEXT_IBM_424_HEBREW_BULLETIN_HPP
#define ZTD_TEXT_IBM_424_HEBREW_BULLETIN_HPP

#include <ztd/text/version.hpp>

#include <ztd/text/unicode_code_point.hpp>
#include <ztd/text/impl/single_byte_lookup_encoding.hpp>

#include <ztd/encoding_tables/ibm_424_hebrew_bulletin.tables.hpp>

#include <ztd/prologue.hpp>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	//////
	/// @brief The Hebrew encoding that matches IBM's Codepage 424.
	///
	/// @tparam _CodeUnit The default code unit type to use when outputting encoded units.
	/// @tparam _CodePoint The code point type to use when outputting decoded units.
	template <typename _CodeUnit = char, typename _CodePoint = unicode_code_point>
	class basic_ibm_424_hebrew_bulletin
	: public __txt_impl::__single_byte_lookup_encoding<basic_ibm_424_hebrew_bulletin<_CodeUnit, _CodePoint>,
		  &::ztd::et::ibm_424_hebrew_bulletin_index_to_code_point,
		  &::ztd::et::ibm_424_hebrew_bulletin_code_point_to_index, _CodeUnit, _CodePoint> { };

	//////
	/// @brief An instance of basic_ibm_8_424ebrew for ease of use.
	inline constexpr basic_ibm_424_hebrew_bulletin<char> ibm_424_hebrew_bulletin = {};

	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text


#include <ztd/epilogue.hpp>

#endif
