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

#ifndef ZTD_TEXT_CODE_UNIT_HPP
#define ZTD_TEXT_CODE_UNIT_HPP

#include <ztd/text/version.hpp>

#include <ztd/idk/type_traits.hpp>

#include <ztd/prologue.hpp>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	//////
	/// @brief Retrieves the code unit type for the given type.
	template <typename _Type>
	class code_unit {
	public:
		//////
		/// @brief The code unit type for the encoding type.
		using type = typename remove_cvref_t<_Type>::code_unit;
	};

	//////
	/// @brief A typename alias for ztd::text::code_unit.
	template <typename _Type>
	using code_unit_t = typename code_unit<remove_cvref_t<_Type>>::type;

	//////
	/// @brief Gets the maximum number of code units that can be produced by an encoding during an encode operation,
	/// suitable for initializing a automatic storage duration ("stack-allocated") buffer.
	template <typename _Type>
	inline static constexpr ::std::size_t max_code_units_v = remove_cvref_t<_Type>::max_code_units;

	namespace __txt_detail {
		template <typename _Type>
		using __detect_code_unit_type = decltype(typename _Type::code_unit {});

		template <typename, typename = void>
		struct __code_unit_or_void {
			using type = void;
		};

		template <typename _Type>
		struct __code_unit_or_void<_Type, ::std::enable_if_t<is_detected_v<__detect_code_unit_type, _Type>>> {
			using type = code_unit_t<_Type>;
		};

		template <typename _Type>
		using __code_unit_or_void_t = __code_unit_or_void<::ztd::remove_cvref_t<_Type>>;
	} // namespace __txt_detail

	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text

#include <ztd/epilogue.hpp>

#endif
