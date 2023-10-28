// =============================================================================
//
// ztd.text
// Copyright © 2022-2023 JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
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

#ifndef ZTD_TEXT_EXECUTION_HPP
#define ZTD_TEXT_EXECUTION_HPP

#include <ztd/text/version.hpp>

#include <ztd/text/no_encoding.hpp>

#include <ztd/text/impl/execution_cuchar.hpp>
#include <ztd/text/impl/execution_mac_os.hpp>
#include <ztd/text/impl/execution_iconv.hpp>

#include <ztd/prologue.hpp>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	//////
	/// @addtogroup ztd_text_encodings Encodings
	///
	/// @{

	//////
	/// @brief The Encoding that represents the "Execution" (narrow locale-based) encoding. The encoding is
	/// typically associated with the locale, which is tied to the C standard library's setlocale function.
	///
	/// @remarks Use of this type is subject to the C Standard Library or platform defaults. Some locales (such as the
	/// Big5 Hong King Supplementary Character Set (Big5-HKSCS)) are broken due to fundamental design issues in the C
	/// Standard Library and bugs in glibc/musl libc's current locale encoding support. On Apple, this is cuurrently
	/// assumed to be UTF-8 since they do not support the @c \<cuchar\> or @c \<uchar.h\> headers.
	class execution_t : public
#if (ZTD_IS_ON(ZTD_CUCHAR) || ZTD_IS_ON(ZTD_UCHAR_H)) && ZTD_IS_OFF(ZTD_PLATFORM_MAC_OS)
		               __txt_impl::__execution_cuchar
#elif ZTD_IS_ON(ZTD_PLATFORM_MAC_OS)
		               __txt_impl::__execution_mac_os
#elif ZTD_IS_ON(ZTD_PLATFORM_LIBICONV)
		               __txt_impl::__execution_iconv
#else
		               no_encoding_t<char, unicode_code_point>
#error \
     "This platform configuration (no POSIX conversions, no <uchar.h> or <cuchar>) is currently not supported. Please" \
	" report the platform you are on and any other relevant information to the maintainer."
#endif
	{
	};

	//////
	/// @brief An instance of the execution_t type for ease of use.
	inline constexpr execution_t execution = {};

	//////
	/// @}

	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text

#include <ztd/epilogue.hpp>

#endif
