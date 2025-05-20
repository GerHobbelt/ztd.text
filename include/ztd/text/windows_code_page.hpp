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

#ifndef ZTD_TEXT_WINDOWS_CODE_PAGE_HPP
#define ZTD_TEXT_WINDOWS_CODE_PAGE_HPP

#include <ztd/text/version.hpp>

#include <ztd/text/encode_result.hpp>
#include <ztd/text/decode_result.hpp>
#include <ztd/text/error_handler.hpp>
#include <ztd/text/utf16.hpp>
#include <ztd/text/unicode_code_point.hpp>
#include <ztd/text/is_ignorable_error_handler.hpp>

#include <ztd/ranges/range.hpp>
#include <ztd/idk/span.hpp>
#include <ztd/idk/type_traits.hpp>
#include <ztd/idk/detail/windows.hpp>

#include <utility>
#include <optional>
#include <cstdint>

#include <ztd/prologue.hpp>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	template <typename _CodeUnit, typename _CodePoint>
	class basic_windows_code_page;

	//////
	/// @brief The encoding representing a Windows conversion using WideCharToMultiByte (encode) and
	/// MultiByteToWideChar (decode) conversion sequences.
	///
	/// @remarks This is slow due to the bad design of WideCharToMultiByte/MultiByteToWideChar, but it does guarantee
	/// access to all of the wide variety of legacy encodings Windows supports. If possible, a different named encoding
	/// type should be used to avoid the performance penalties that comes from API limitations of this encoding object.
	/// Occasionally, for correct text, this API limitations can be mitigated when using the bulk APIs: for the
	/// single-conversion APIs that do not perform bulk conversion, the performance penalty is endemic.
	template <typename _CodeUnit, typename _CodePoint = unicode_code_point>
	class basic_windows_code_page {
	public:
		//////
		/// @brief The code point type that is decoded to, and encoded from.
		using code_point = _CodePoint;
		//////
		/// @brief The code unit type that is decoded from, and encoded to.
		using code_unit = _CodeUnit;

		//////
		/// @brief The associated state for decode operations.
		struct decode_state {
			std::optional<code_point> maybe_replacement_code_point;

			constexpr decode_state() : maybe_replacement_code_point() {
			}

			constexpr decode_state(code_point __replacement_code_point)
			: maybe_replacement_code_point(__replacement_code_point) {
			}

			constexpr decode_state(const basic_windows_code_page& __encoding)
			: maybe_replacement_code_point(__encoding._M_replacement_code_point) {
			}
		};

		//////
		/// @brief The associated state for encode operations.
		struct encode_state {
			std::optional<code_unit> maybe_replacement_code_unit;

			constexpr encode_state() : maybe_replacement_code_unit() {
			}

			constexpr encode_state(code_point __replacement_code_unit)
			: maybe_replacement_code_unit(__replacement_code_unit) {
			}

			constexpr encode_state(const basic_windows_code_page& __encoding)
			: maybe_replacement_code_unit(__encoding._M_replacement_code_unit) {
			}
		};

		//////
		/// @brief Returns a potential view into the code points to use when an error happens and a replacement
		/// strategy is being used.
		///
		/// @param[in] __state The decode state of this particular conversion operation.
		///
		/// @return A potential view of code points to do replacements with. This is influenced by either what the
		/// state for the conversion was initialized with, and it defaults to what was stored in the actual encoding
		/// object.
		constexpr ::std::optional<::ztd::span<const code_point>> maybe_replacement_code_points(
			const decode_state& __state) noexcept {
			if (__state.maybe_replacement_code_point.has_value()) {
				return ::std::optional<::ztd::span<const code_point>>(
					::std::in_place, ::std::addressof(__state.maybe_replacement_code_point.value()), 1);
			}
			return ::std::nullopt;
		}

		//////
		/// @brief Returns a potential view into the code points to use when an error happens and a replacement
		/// strategy is being used.
		///
		/// @param[in] __state The encode state of this particular conversion operation.
		///
		/// @return A potential view of code units to do replacements with. This is influenced by either what the
		/// state for the conversion was initialized with, and it defaults to what was stored in the actual encoding
		/// object.
		constexpr ::std::optional<::ztd::span<const code_unit>> maybe_replacement_code_units(
			const encode_state& __state) noexcept {
			if (__state.maybe_replacement_code_unit.has_value()) {
				return ::std::optional<::ztd::span<const code_point>>(
					::std::in_place, ::std::addressof(__state.maybe_replacement_code_unit.value()), 1);
			}
			return ::std::nullopt;
		}

		//////
		/// @brief The maximum code units a single complete operation of encoding can produce.
		inline static constexpr const ::std::size_t max_code_units = 8;
		//////
		/// @brief The maximum number of code points a single complete operation of decoding can produce.
		inline static constexpr const ::std::size_t max_code_points = 8;


		//////
		/// @brief Default constructor: assumes the code page is the default (thread) code page with the value
		/// `CP_THREAD_ACP`.
		constexpr basic_windows_code_page() noexcept
		: basic_windows_code_page(::ztd::__idk_detail::__windows::__code_page_active_thread) {
		}

		//////
		/// @brief Constructor that applies the given code page to the underlying conversions.
		///
		/// @param __win32_code_page The code page. It is one of the allowed code page values
		/// (https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers) that are blessed by the
		/// WideCharToMultiByte and MultiByteToWideChar.
		constexpr basic_windows_code_page(int32_t __win32_code_page) noexcept
		: _M_code_page(__win32_code_page), _M_replacement_code_point(), _M_replacement_code_unit() {
		}

		//////
		/// @brief Constructor that applies the given code page to the conversions, and sets the default-used
		/// replacement code point and replacement code unit for the underlying algorithm where possible.
		///
		/// @param __win32_code_page The code page. It is one of the allowed code page values
		/// (https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers) that are blessed by the
		/// WideCharToMultiByte and MultiByteToWideChar.
		///
		/// @param[in] __replacement_code_point The replacement code point (a single 2-byte UTF-16 code unit) to be
		/// used with the underlying algorithm, where permissible.
		///
		/// @param[in] __replacement_code_unit The replacement code unit (a single 1-byte `__win32_code_page`-encoded
		/// code unit) to be used with the underlying algorithm, where permissible.
		constexpr basic_windows_code_page(
			int32_t __win32_code_page, uint16_t __replacement_code_point, int8_t __replacement_code_unit) noexcept
		: _M_code_page(__win32_code_page)
		, _M_replacement_code_point(static_cast<code_point>(__replacement_code_point))
		, _M_replacement_code_unit(static_cast<code_unit>(__replacement_code_unit)) {
		}

		//////
		/// @brief Decodes a single complete unit of information as code points and produces a result with the
		/// input and output ranges moved past what was successfully read and written; or, produces an error and
		/// returns the input and output ranges untouched.
		///
		/// @param[in] __input The input view to read code uunits from.
		/// @param[in] __output The output view to write code points into.
		/// @param[in] __error_handler The error handler to invoke if encoding fails.
		/// @param[in, out] __s The necessary state information. Most encodings have no state, but because this
		/// is effectively a runtime encoding and therefore it is important to preserve and manage this state.
		///
		/// @returns A ztd::text::decode_result object that contains the reconstructed input range,
		/// reconstructed output range, error handler, and a reference to the passed-in state.
		template <typename _Input, typename _Output, typename _ErrorHandler>
		auto decode_one(
			_Input&& __input, _Output&& __output, _ErrorHandler&& __error_handler, decode_state& __s) const {
#if ZTD_IS_ON(ZTD_PLATFORM_WINDOWS)
			using _UErrorHandler                = remove_cvref_t<_ErrorHandler>;
			using _SubInput                     = ztd::ranges::subrange_for_t<::std::remove_reference_t<_Input>>;
			using _SubOutput                    = ztd::ranges::subrange_for_t<::std::remove_reference_t<_Output>>;
			using _Result                       = decode_result<_SubInput, _SubOutput, decode_state>;
			constexpr bool __call_error_handler = !is_ignorable_error_handler_v<_UErrorHandler>;

			auto __in_it   = ::ztd::ranges::begin(__input);
			auto __in_last = ::ztd::ranges::end(__input);
			if (__in_it == __in_last) {
				// an exhausted sequence is fine
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					::std::forward<_Output>(__output), __s, encoding_error::ok);
			}


			auto __out_it   = ::ztd::ranges::begin(__output);
			auto __out_last = ::ztd::ranges::end(__output);

			using __wutf16             = __txt_impl::__utf16_with<void, wchar_t, code_point, false>;
			using __intermediate_state = encode_state_t<__wutf16>;

			__wutf16 __intermediate_encoding {};
			__intermediate_state __intermediate_s {};
			__txt_detail::__progress_handler<!__call_error_handler, __wutf16> __intermediate_handler {};
			wchar_t __wide_intermediary[8] {};
			::ztd::span<wchar_t> __wide_write_buffer(__wide_intermediary);
			auto __intermediate_result = __intermediate_encoding.encode_one(
				::std::forward<_Input>(__input), __wide_write_buffer, __intermediate_handler, __intermediate_s);
			if constexpr (__call_error_handler) {
				if (__intermediate_result.error_code != encoding_error::ok) {
					return ::std::forward<_ErrorHandler>(__error_handler)(*this,
						_Result(::std::move(__intermediate_result.input), ::std::forward<_Output>(__output), __s,
						     __intermediate_result.error_code),
						__intermediate_handler._M_code_points_progress(), ::ztd::span<code_unit, 0>());
				}
			}
			constexpr const ::std::size_t __state_count_max = 12;
			code_unit __intermediary_output[__state_count_max] {};
			int __used_default_char = false;
			::ztd::span<const wchar_t> __wide_read_buffer(__wide_intermediary, __intermediate_result.output.data());
			int __res = ::WideCharToMultiByte(this->_M_code_page, WC_ERR_INVALID_CHARS, __wide_read_buffer.data(),
				static_cast<int>(__wide_read_buffer.size()), __intermediary_output, __state_count_max, nullptr,
				::std::addressof(__used_default_char));
			if constexpr (__call_error_handler) {
				if (__res == 0) {
					return ::std::forward<_ErrorHandler>(__error_handler)(*this,
						_Result(::std::move(__intermediate_result.input), ::std::forward<_Output>(__output), __s,
						     ::GetLastError() == ERROR_INSUFFICIENT_BUFFER
						          ? encoding_error::insufficient_output_space
						          : encoding_error::invalid_sequence),
						__intermediate_handler._M_code_points_progress(), ::ztd::span<code_unit, 0>());
				}
			}
			for (auto __intermediary_it = __intermediary_output; __res-- > 0;) {
				if constexpr (__call_error_handler) {
					if (__out_it == __out_last) {
						::ztd::span<code_unit> __code_unit_progress(
							__intermediary_it, static_cast<::std::size_t>(__res + 1));
						return ::std::forward<_ErrorHandler>(__error_handler)(*this,
							_Result(::std::move(__intermediate_result.input),
							     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __s,
							     encoding_error::insufficient_output_space),
							__intermediate_handler._M_code_points_progress(), __code_unit_progress);
					}
				}
				*__out_it = *__intermediary_it;
				::ztd::ranges::iter_advance(__out_it);
			}
			return _Result(::std::move(__intermediate_result.input),
				_SubOutput(::std::move(__out_it), ::std::move(__out_last)), __s, __intermediate_result.error_code);
#else
			(void)__input;
			(void)__output;
			(void)__error_handler;
			(void)__s;
			static_assert(::ztd::always_false_v<_Input>,
				"This encoding type cannot be used on non-Windows platforms; please consider swapping this out for "
				"a different encoding type for a non-Windows platform.");
#endif
		}

		//////
		/// @brief Encodes a single complete unit of information as code units and produces a result with the
		/// input and output ranges moved past what was successfully read and written; or, produces an error and
		/// returns the input and output ranges untouched.
		///
		/// @param[in] __input The input view to read code uunits from.
		/// @param[in] __output The output view to write code points into.
		/// @param[in] __error_handler The error handler to invoke if encoding fails.
		/// @param[in, out] __s The necessary state information. Most encodings have no state, but because this
		/// is effectively a runtime encoding and therefore it is important to preserve and manage this state.
		///
		/// @returns A ztd::text::encode_result object that contains the reconstructed input range,
		/// reconstructed output range, error handler, and a reference to the passed-in state.
		template <typename _Input, typename _Output, typename _ErrorHandler>
		auto encode_one(
			_Input&& __input, _Output&& __output, _ErrorHandler&& __error_handler, encode_state& __s) const {
#if ZTD_IS_ON(ZTD_PLATFORM_WINDOWS)
			using _UErrorHandler                = remove_cvref_t<_ErrorHandler>;
			using _SubInput                     = ztd::ranges::subrange_for_t<::std::remove_reference_t<_Input>>;
			using _SubOutput                    = ztd::ranges::subrange_for_t<::std::remove_reference_t<_Output>>;
			using _Result                       = encode_result<_SubInput, _SubOutput, encode_state>;
			constexpr bool __call_error_handler = !is_ignorable_error_handler_v<_UErrorHandler>;

			auto __in_it   = ::ztd::ranges::begin(__input);
			auto __in_last = ::ztd::ranges::end(__input);
			if (__in_it == __in_last) {
				// an exhausted sequence is fine
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					::std::forward<_Output>(__output), __s, encoding_error::ok);
			}

			auto __out_it   = ::ztd::ranges::begin(__output);
			auto __out_last = ::ztd::ranges::end(__output);

			using __wutf16             = __txt_impl::__utf16_with<void, wchar_t, code_point, false>;
			using __intermediate_state = encode_state_t<__wutf16>;

			__wutf16 __intermediate_encoding {};
			__intermediate_state __intermediate_s {};
			__txt_detail::__progress_handler<!__call_error_handler, __wutf16> __intermediate_handler {};
			wchar_t __wide_intermediary[8] {};
			::ztd::span<wchar_t> __wide_write_buffer(__wide_intermediary);
			auto __intermediate_result = __intermediate_encoding.encode_one(
				::std::forward<_Input>(__input), __wide_write_buffer, __intermediate_handler, __intermediate_s);
			if constexpr (__call_error_handler) {
				if (__intermediate_result.error_code != encoding_error::ok) {
					return ::std::forward<_ErrorHandler>(__error_handler)(*this,
						_Result(::std::move(__intermediate_result.input), ::std::forward<_Output>(__output), __s,
						     __intermediate_result.error_code),
						__intermediate_handler._M_code_points_progress(), ::ztd::span<code_unit, 0>());
				}
			}
			constexpr const ::std::size_t __state_count_max = 12;
			code_unit __intermediary_output[__state_count_max] {};
			BOOL __used_default_char = false;
			::ztd::span<const wchar_t> __wide_read_buffer(__wide_intermediary, __intermediate_result.output.data());
			int __res = ::WideCharToMultiByte(static_cast<UINT>(this->code_page()), WC_ERR_INVALID_CHARS,
				__wide_read_buffer.data(), static_cast<int>(__wide_read_buffer.size()), __intermediary_output,
				__state_count_max, nullptr, ::std::addressof(__used_default_char));
			if constexpr (__call_error_handler) {
				if (__res == 0) {
					return ::std::forward<_ErrorHandler>(__error_handler)(*this,
						_Result(::std::move(__intermediate_result.input), ::std::forward<_Output>(__output), __s,
						     ::GetLastError() == ERROR_INSUFFICIENT_BUFFER
						          ? encoding_error::insufficient_output_space
						          : encoding_error::invalid_sequence),
						__intermediate_handler._M_code_points_progress(), ::ztd::span<code_unit, 0>());
				}
			}
			for (auto __intermediary_it = __intermediary_output; __res-- > 0;) {
				if constexpr (__call_error_handler) {
					if (__out_it == __out_last) {
						::ztd::span<code_unit> __code_unit_progress(
							__intermediary_it, static_cast<::std::size_t>(__res + 1));
						return ::std::forward<_ErrorHandler>(__error_handler)(*this,
							_Result(::std::move(__intermediate_result.input),
							     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __s,
							     encoding_error::insufficient_output_space),
							__intermediate_handler._M_code_points_progress(), __code_unit_progress);
					}
				}
				*__out_it = *__intermediary_it;
				::ztd::ranges::iter_advance(__out_it);
			}
			return _Result(::std::move(__intermediate_result.input),
				_SubOutput(::std::move(__out_it), ::std::move(__out_last)), __s, __intermediate_result.error_code);
#else
			(void)__input;
			(void)__output;
			(void)__error_handler;
			(void)__s;
			static_assert(::ztd::always_false_v<_Input>,
				"This encoding type cannot be used on non-Windows platforms; please consider swapping this out for "
				"a different encoding type for a non-Windows platform.");
#endif
		}

		constexpr int32_t code_page() const noexcept {
			return this->_M_code_page;
		}

		constexpr ::std::optional<code_point> maybe_default_replacement_code_point() const noexcept {
			return this->_M_replacement_code_point;
		}

		constexpr ::std::optional<code_unit> maybe_default_replacement_code_unit() const noexcept {
			return this->_M_replacement_code_unit;
		}

	private:
		int32_t _M_code_page;
		::std::optional<code_point> _M_replacement_code_point;
		::std::optional<code_unit> _M_replacement_code_unit;
	};

	using windows_code_page = basic_windows_code_page<char, unicode_code_point>;

	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text

#include <ztd/epilogue.hpp>

#endif
