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

#ifndef ZTD_TEXT_DECODE_HPP
#define ZTD_TEXT_DECODE_HPP

#include <ztd/text/version.hpp>

#include <ztd/text/code_point.hpp>
#include <ztd/text/default_encoding.hpp>
#include <ztd/text/decode_result.hpp>
#include <ztd/text/error_handler.hpp>
#include <ztd/text/state.hpp>
#include <ztd/text/is_unicode_code_point.hpp>
#include <ztd/text/max_units.hpp>
#include <ztd/text/detail/span_reconstruct.hpp>
#include <ztd/text/detail/is_lossless.hpp>
#include <ztd/text/detail/encoding_range.hpp>
#include <ztd/text/detail/forward_if_move_only.hpp>
#include <ztd/text/detail/update_input.hpp>

#include <ztd/idk/span.hpp>
#include <ztd/idk/type_traits.hpp>
#include <ztd/idk/char_traits.hpp>
#include <ztd/idk/tag.hpp>
#include <ztd/ranges/unbounded.hpp>
#include <ztd/ranges/detail/insert_bulk.hpp>

#include <string>
#include <vector>
#include <utility>

#include <ztd/prologue.hpp>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	//////
	/// @addtogroup ztd_text_decode ztd::text::decode[_into]
	///
	/// @brief These functions convert from a view of input into a view of output using either the inferred or
	/// specified encodings. If no error handler is provided, the equivalent of the ztd::text::default_handler_t is
	/// used by default. If no associated state is provided for either the "to" or "from" encodings, one will be
	/// created with automatic storage duration (as a "stack" variable) for the provided encoding.
	/// @{

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from
	/// the intermediate code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the `__encoding` 's decode step.
	///
	/// @result A ztd::text::decode_result object that contains references to `__state`.
	///
	/// @remarks This function performs the bog-standard, basic loop for decoding. It talks to no ADL extension
	/// points.
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler, typename _State>
	constexpr auto basic_decode_into_raw(_Input&& __input, _Encoding&& __encoding, _Output&& __output,
		_ErrorHandler&& __error_handler, _State& __state) {
		using _UEncoding     = remove_cvref_t<_Encoding>;
		using _UErrorHandler = remove_cvref_t<_ErrorHandler>;

		static_assert(__txt_detail::__is_decode_lossless_or_deliberate_v<_UEncoding, _UErrorHandler>,
			ZTD_TEXT_LOSSY_DECODE_MESSAGE_I_);

		::std::size_t __error_count = 0;

		auto __first_result = __encoding.decode_one(
			::std::forward<_Input>(__input), ::std::forward<_Output>(__output), __error_handler, __state);
		using _WorkingInput  = decltype(__first_result.input);
		using _WorkingOutput = decltype(__first_result.output);
		using _Result        = decode_result<_WorkingInput, _WorkingOutput, _State>;
		if (__first_result.error_code != encoding_error::ok) {
			return _Result(::std::move(__first_result.input), ::std::move(__first_result.output), __state,
				__first_result.error_code, __error_count);
		}
		__error_count += __first_result.error_count;
		if (::ztd::ranges::empty(__first_result.input)) {
			if (::ztd::text::is_state_complete(__encoding, __state)) {
				return _Result(::std::move(__first_result.input), ::std::move(__first_result.output), __state,
					encoding_error::ok, __error_count);
			}
		}

		_WorkingInput __working_input   = ::std::move(__first_result.input);
		_WorkingOutput __working_output = ::std::move(__first_result.output);

		for (;;) {
			auto __result = __encoding.decode_one(
				::std::move(__working_input), ::std::move(__working_output), __error_handler, __state);
			__error_count += __result.error_count;
			__working_input  = ::std::move(__result.input);
			__working_output = ::std::move(__result.output);
			if (__result.error_code != encoding_error::ok) {
				return _Result(::std::move(__working_input), ::std::move(__working_output), __state,
					__result.error_code, __error_count);
			}
			if (::ztd::ranges::empty(__working_input)) {
				if (!::ztd::text::is_state_complete(__encoding, __state)) {
					continue;
				}
				break;
			}
		}
		return _Result(
			::std::move(__working_input), ::std::move(__working_output), __state, encoding_error::ok, __error_count);
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the `__encoding` 's decode step.
	///
	/// @result A ztd::text::decode_result object that contains references to `__state`.
	///
	/// @remarks This function detects whether or not the ADL extension point `text_decode` can be called with the
	/// provided parameters. If so, it will use that ADL extension point over the default implementation. Otherwise, it
	/// will loop over the two encodings and attempt to decode by repeatedly calling the encoding's required @c
	/// decode_one function.
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler, typename _State>
	constexpr auto decode_into_raw(_Input&& __input, _Encoding&& __encoding, _Output&& __output,
		_ErrorHandler&& __error_handler, _State& __state) {
		if constexpr (is_detected_v<__txt_detail::__detect_adl_text_decode, _Input, _Encoding, _Output, _ErrorHandler,
			              _State>) {
			return text_decode(::ztd::tag<remove_cvref_t<_Encoding>> {}, ::std::forward<_Input>(__input),
				::std::forward<_Encoding>(__encoding), ::std::forward<_Output>(__output),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else if constexpr (is_detected_v<__txt_detail::__detect_adl_internal_text_decode, _Input, _Encoding, _Output,
			                   _ErrorHandler, _State>) {
			return __text_decode(::ztd::tag<remove_cvref_t<_Encoding>> {}, ::std::forward<_Input>(__input),
				::std::forward<_Encoding>(__encoding), ::std::forward<_Output>(__output),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else {
			return basic_decode_into_raw(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_Output>(__output), ::std::forward<_ErrorHandler>(__error_handler), __state);
		}
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to `__state`.
	///
	/// @remarks Creates a default `state` using ztd::text::make_decode_state.
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler>
	constexpr auto decode_into_raw(
		_Input&& __input, _Encoding&& __encoding, _Output&& __output, _ErrorHandler&& __error_handler) {
		using _UEncoding = remove_cvref_t<_Encoding>;
		using _State     = decode_state_t<_UEncoding>;

		_State __state = ::ztd::text::make_decode_state(__encoding);
		auto __stateful_result
			= ::ztd::text::decode_into_raw(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			     ::std::forward<_Output>(__output), ::std::forward<_ErrorHandler>(__error_handler), __state);
		return __txt_detail::__slice_to_stateless_decode(::std::move(__stateful_result));
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to `__state`.
	///
	/// @remarks Creates a default `error_handler` that is similar to ztd::text::default_handler_t, but marked as
	/// careless.
	template <typename _Input, typename _Encoding, typename _Output>
	constexpr auto decode_into_raw(_Input&& __input, _Encoding&& __encoding, _Output&& __output) {
		default_handler_t __handler {};
		return ::ztd::text::decode_into_raw(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_Output>(__output), __handler);
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to `__state`.
	///
	/// @remarks Creates a default `encoding` by figuring out the `value_type` of the `__input`, then passing that
	/// type into ztd::text::default_code_point_encoding_t. That encoding is that used to decode the input code units,
	/// by default.
	template <typename _Input, typename _Output>
	constexpr auto decode_into_raw(_Input&& __input, _Output&& __output) {
		using _UInput   = remove_cvref_t<_Input>;
		using _CodeUnit = ranges::range_value_type_t<_UInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (::std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using _Encoding = default_consteval_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode_into_raw(
				::std::forward<_Input>(__input), __encoding, ::std::forward<_Output>(__output));
		}
		else
#endif
		{
			using _Encoding = default_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode_into_raw(
				::std::forward<_Input>(__input), __encoding, ::std::forward<_Output>(__output));
		}
	}

	namespace __txt_detail {
		template <typename _Input, typename _Encoding, typename _OutputContainer, typename _ErrorHandler,
			typename _State>
		constexpr auto __intermediate_decode_to_storage(_Input&& __input, _Encoding&& __encoding,
			_OutputContainer& __output, _ErrorHandler&& __error_handler, _State& __state) {
			// Well, SHIT. Write into temporary, then serialize one-by-one/bulk to output.
			// I'll admit, this is HELLA work to support...
			using _UEncoding                    = remove_cvref_t<_Encoding>;
			using _UErrorHandler                = remove_cvref_t<_ErrorHandler>;
			constexpr ::std::size_t __max_units = max_decode_code_points_v<_UEncoding>;
			constexpr ::std::size_t __intermediate_buffer_max
				= ZTD_TEXT_INTERMEDIATE_DECODE_BUFFER_SIZE_I_(code_point_t<_UEncoding>) < __max_units
				? __max_units
				: ZTD_TEXT_INTERMEDIATE_DECODE_BUFFER_SIZE_I_(code_point_t<_UEncoding>);
			using _IntermediateValueType = code_point_t<_UEncoding>;
			using _IntermediateInput     = __txt_detail::__span_reconstruct_t<_Input, _Input>;
			using _InitialOutput         = ::ztd::span<_IntermediateValueType, __intermediate_buffer_max>;
			using _Output                = ::ztd::span<_IntermediateValueType>;
			using _Result                = decltype(__encoding.decode_one(
                    ::std::declval<_IntermediateInput>(), ::std::declval<_Output>(), __error_handler, __state));
			using _ResultInput           = decltype(::std::declval<_Result>().input);
			using _WorkingInput          = __span_reconstruct_t<_ResultInput, _ResultInput>;

			static_assert(__txt_detail::__is_decode_lossless_or_deliberate_v<_Encoding, _ErrorHandler>,
				ZTD_TEXT_LOSSY_DECODE_MESSAGE_I_);

			// We MUST use a temporary error handler
			// as well as a pass-throuugh handler if we end up with lots of intermediary input
			__progress_handler<is_ignorable_error_handler_v<_UErrorHandler>, _UEncoding> __intermediate_handler {};

			_IntermediateInput __intermediate_input
				= __txt_detail::__span_reconstruct<_Input>(::std::forward<_Input>(__input));
			_WorkingInput __working_input(::std::move(__intermediate_input));
			_IntermediateValueType __intermediate_translation_buffer[__intermediate_buffer_max] {};

			for (;;) {
				// Ignore "out of output" errors and do our best to recover properly along the way...
				_InitialOutput __intermediate_initial_output(__intermediate_translation_buffer);
				auto __result = ::ztd::text::decode_into_raw(::std::move(__working_input), __encoding,
					__intermediate_initial_output, __intermediate_handler, __state);
				_Output __intermediate_output(__intermediate_initial_output.data(), __result.output.data());
				ranges::__rng_detail::__container_insert_bulk(__output, __intermediate_output);
				if (__result.error_code == encoding_error::insufficient_output_space) {
					if (__intermediate_handler._M_code_points_progress_size() != 0) {
						// add any leftover partially-unwritten characters to our output
						ranges::__rng_detail::__container_insert_bulk(
							__output, __intermediate_handler._M_const_code_points_progress());
						// it's okay, just loop around, we've got S P A C E for more
						__working_input
							= __txt_detail::__update_input<_WorkingInput>(::std::move(__result.input));
					}
					else if (__intermediate_handler._M_code_units_progress_size() != 0) {
						if constexpr (::ztd::ranges::is_range_bidirectional_range_v<_WorkingInput>) {
							// we can try to rewind our current input by the amount that was not successfully
							// read. This will allow us to try again, when the buffer
							// has more space in it, and should not result in the same error,
							// unless it was legitimiately an encoding_error::invalid_sequence.
							__working_input = __txt_detail::__update_input<_WorkingInput>(
								::ztd::ranges::reconstruct(::std::in_place_type<_WorkingInput>,
								     ztd::ranges::iter_recede(ztd::ranges::begin(__result.input),
								          __intermediate_handler._M_code_units_progress_size()),
								     ztd::ranges::end(__result.input)));
						}
						else {
							// this is an effectively-impossible case, as we cannot stitch the old input together
							// with the current input.
							// simply bail!!
							return __result;
						}
					}
					else {
						// it's okay, just loop around, we've got S P A C E for more
						__working_input
							= __txt_detail::__update_input<_WorkingInput>(::std::move(__result.input));
					}
					__intermediate_handler.clear();
					continue;
				}
				if (__result.error_code != encoding_error::ok) {
					// mill result through actual error handler!
					auto __error_result
						= ::std::forward<_ErrorHandler>(__error_handler)(::std::forward<_Encoding>(__encoding),
						     ::std::move(__result), __intermediate_handler._M_code_units_progress(),
						     __intermediate_handler._M_code_points_progress());
					return __error_result;
				}
				if (::ztd::ranges::empty(__result.input)) {
					if (!::ztd::text::is_state_complete(__encoding, __state)) {
						continue;
					}
					return __result;
				}
			}
		}

		template <bool _OutputOnly, bool _NoState, typename _OutputContainer, typename _Input, typename _Encoding,
			typename _ErrorHandler, typename _State>
		constexpr auto __decode_dispatch(
			_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler, _State& __state) {
			using _UEncoding                    = remove_cvref_t<_Encoding>;
			constexpr ::std::size_t __max_units = max_decode_code_points_v<_UEncoding>;

			_OutputContainer __output {};
			if constexpr (is_detected_v<ranges::detect_adl_size, _Input>) {
				using _SizeType = decltype(::ztd::ranges::size(__input));
				if constexpr (is_detected_v<ranges::detect_reserve_with_size, _OutputContainer, _SizeType>) {
					auto __output_size_hint = ::ztd::ranges::size(__input);
					__output_size_hint *= (__max_units / 2);
					__output.reserve(__output_size_hint);
				}
			}
			auto __stateful_result = __txt_detail::__intermediate_decode_to_storage(::std::forward<_Input>(__input),
				::std::forward<_Encoding>(__encoding), __output, ::std::forward<_ErrorHandler>(__error_handler),
				__state);
			if constexpr (_OutputOnly) {
				(void)__stateful_result;
				return __output;
			}
			else if constexpr (_NoState) {
				return __txt_detail::__replace_decode_result_output_no_state(
					::std::move(__stateful_result), ::std::move(__output));
			}
			else {
				return __txt_detail::__replace_decode_result_output(
					::std::move(__stateful_result), ::std::move(__output));
			}
		}

	} // namespace __txt_detail

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from
	/// the intermediate code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the `__encoding` 's decode step.
	///
	/// @result A ztd::text::decode_result object that contains references to `__state`.
	///
	/// @remarks This function performs the bog-standard, basic loop for decoding. It talks to no ADL extension
	/// points.
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler, typename _State>
	constexpr auto basic_decode_into(_Input&& __input, _Encoding&& __encoding, _Output&& __output,
		_ErrorHandler&& __error_handler, _State& __state) {
		auto __reconstructed_input       = __txt_detail::__span_reconstruct<_Input>(::std::forward<_Input>(__input));
		auto __result                    = basic_decode_into_raw(::std::move(__reconstructed_input),
			                   ::std::forward<_Encoding>(__encoding), ::std::forward<_Output>(__output), __error_handler, __state);
		using _ReconstructedResultInput  = __txt_detail::__span_reconstruct_t<_Input, _Input>;
		using _ReconstructedResultOutput = __txt_detail::__span_reconstruct_mutable_t<_Output, _Output>;
		using _Result = decode_result<_ReconstructedResultInput, _ReconstructedResultOutput, _State>;
		return _Result(__txt_detail::__span_reconstruct<_Input>(::std::move(__result.input)),
			__txt_detail::__span_reconstruct_mutable<_Output>(::std::move(__result.output)), __result.state);
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from
	/// the intermediate code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the `__encoding` 's decode step.
	///
	/// @result A ztd::text::decode_result object that contains references to `__state`.
	///
	/// @remarks This function performs the bog-standard, basic loop for decoding. It talks to no ADL extension
	/// points.
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler, typename _State>
	constexpr auto decode_into(_Input&& __input, _Encoding&& __encoding, _Output&& __output,
		_ErrorHandler&& __error_handler, _State& __state) {
		auto __reconstructed_input       = __txt_detail::__span_reconstruct<_Input>(::std::forward<_Input>(__input));
		auto __result                    = ::ztd::text::decode_into_raw(::std::move(__reconstructed_input),
			                   ::std::forward<_Encoding>(__encoding), ::std::forward<_Output>(__output), __error_handler, __state);
		using _ReconstructedResultInput  = __txt_detail::__span_reconstruct_t<_Input, _Input>;
		using _ReconstructedResultOutput = __txt_detail::__span_reconstruct_mutable_t<_Output, _Output>;
		using _Result = decode_result<_ReconstructedResultInput, _ReconstructedResultOutput, _State>;
		return _Result(__txt_detail::__span_reconstruct<_Input>(::std::move(__result.input)),
			__txt_detail::__span_reconstruct_mutable<_Output>(::std::move(__result.output)), __result.state);
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to `__state`.
	///
	/// @remarks Creates a default `state` using ztd::text::make_decode_state.
	template <typename _Input, typename _Encoding, typename _Output, typename _ErrorHandler>
	constexpr auto decode_into(
		_Input&& __input, _Encoding&& __encoding, _Output&& __output, _ErrorHandler&& __error_handler) {
		using _UEncoding = remove_cvref_t<_Encoding>;
		using _State     = decode_state_t<_UEncoding>;

		_State __state = ::ztd::text::make_decode_state(__encoding);
		auto __stateful_result
			= ::ztd::text::decode_into(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			     ::std::forward<_Output>(__output), ::std::forward<_ErrorHandler>(__error_handler), __state);
		return __txt_detail::__slice_to_stateless_decode(::std::move(__stateful_result));
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to `__state`.
	///
	/// @remarks Creates a default `error_handler` that is similar to ztd::text::default_handler_t, but marked as
	/// careless.
	template <typename _Input, typename _Encoding, typename _Output>
	constexpr auto decode_into(_Input&& __input, _Encoding&& __encoding, _Output&& __output) {
		default_handler_t __handler {};
		return ::ztd::text::decode_into(::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
			::std::forward<_Output>(__output), __handler);
	}

	//////
	/// @brief Converts from the code units of the given `__input` view through the encoding to code points into the
	/// `__output` view.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __output An output_view to write code points to as the result of the decode operation from the
	/// intermediate code units.
	///
	/// @result A ztd::text::stateless_decode_result object that contains references to `__state`.
	///
	/// @remarks Creates a default `encoding` by figuring out the `value_type` of the `__input`, then passing that
	/// type into ztd::text::default_code_point_encoding_t. That encoding is that used to decode the input code units,
	/// by default.
	template <typename _Input, typename _Output>
	constexpr auto decode_into(_Input&& __input, _Output&& __output) {
		using _UInput   = remove_cvref_t<_Input>;
		using _CodeUnit = ranges::range_value_type_t<_UInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (::std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using _Encoding = default_consteval_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode_into(
				::std::forward<_Input>(__input), __encoding, ::std::forward<_Output>(__output));
		}
		else
#endif
		{
			using _Encoding = default_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode_into(
				::std::forward<_Input>(__input), __encoding, ::std::forward<_Output>(__output));
		}
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the `__encoding` 's decode step.
	///
	/// @result A ztd::text::decode_result object that contains references to `__state` and an output of type
	/// `_OutputContainer`.
	///
	/// @remarks This function detects creates a container of type `_OutputContainer` and uses a typical @c
	/// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to. The result is
	/// then returned, with the `.output` value put into the container.
	template <typename _OutputContainer = void, typename _Input, typename _Encoding, typename _ErrorHandler,
		typename _State>
	constexpr auto decode_to(
		_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler, _State& __state) {
		using _UEncoding                = remove_cvref_t<_Encoding>;
		using _UOutputContainer         = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint          = code_point_t<_UEncoding>;
		constexpr bool _IsVoidContainer = ::std::is_void_v<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || is_unicode_code_point_v<_OutputCodePoint>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			// prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
			using _RealOutputContainer = ::std::basic_string<_OutputCodePoint>;
			return __txt_detail::__decode_dispatch<false, false, _RealOutputContainer>(
				::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else {
			using _RealOutputContainer
				= ::std::conditional_t<_IsVoidContainer, ::std::vector<_OutputCodePoint>, _OutputContainer>;
			return __txt_detail::__decode_dispatch<false, false, _RealOutputContainer>(
				::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result A ztd::text::stateless_decode_result object whose output is of type `_OutputContainer`.
	///
	/// @remarks This function creates a `state` using ztd::text::make_decode_state.
	template <typename _OutputContainer = void, typename _Input, typename _Encoding, typename _ErrorHandler>
	constexpr auto decode_to(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler) {
		using _UEncoding                = remove_cvref_t<_Encoding>;
		using _UOutputContainer         = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint          = code_point_t<_UEncoding>;
		using _State                    = decode_state_t<_UEncoding>;
		_State __state                  = ::ztd::text::make_decode_state(__encoding);
		constexpr bool _IsVoidContainer = ::std::is_void_v<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || is_unicode_code_point_v<_OutputCodePoint>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			// prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
			using _RealOutputContainer = ::std::basic_string<_OutputCodePoint>;
			return __txt_detail::__decode_dispatch<false, true, _RealOutputContainer>(
				::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else {
			using _RealOutputContainer
				= ::std::conditional_t<_IsVoidContainer, ::std::vector<_OutputCodePoint>, _OutputContainer>;
			return __txt_detail::__decode_dispatch<false, true, _RealOutputContainer>(
				::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	///
	/// @result A ztd::text::stateless_decode_result object whose output is of type `_OutputContainer`.
	///
	/// @remarks This function creates a `handler` using ztd::text::default_handler_t, but marks it as careless.
	template <typename _OutputContainer = void, typename _Input, typename _Encoding>
	constexpr auto decode_to(_Input&& __input, _Encoding&& __encoding) {
		default_handler_t __handler {};
		return ::ztd::text::decode_to<_OutputContainer>(
			::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding), __handler);
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will produce code
	/// points.
	///
	/// @result A ztd::text::stateless_decode_result object whose output is of type `_OutputContainer`.
	///
	/// @remarks This function creates an `encoding` by using the `value_type` of the `__input` which is then
	/// passed through the ztd::text::default_code_point_encoding type to get the default desired encoding.
	template <typename _OutputContainer = void, typename _Input>
	constexpr auto decode_to(_Input&& __input) {
		using _UInput   = remove_cvref_t<_Input>;
		using _CodeUnit = ranges::range_value_type_t<_UInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (::std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using _Encoding = default_consteval_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode_to<_OutputContainer>(::std::forward<_Input>(__input), __encoding);
		}
		else
#endif
		{
			using _Encoding = default_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode_to<_OutputContainer>(::std::forward<_Input>(__input), __encoding);
		}
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	/// @param[in,out] __state A reference to the associated state for the `__encoding` 's decode step.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function detects creates a container of type `_OutputContainer` and uses a typical @c
	/// std::back_inserter or `std::push_back_inserter` to fill in elements as it is written to.
	template <typename _OutputContainer = void, typename _Input, typename _Encoding, typename _ErrorHandler,
		typename _State>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler, _State& __state) {
		using _UEncoding                = remove_cvref_t<_Encoding>;
		using _UOutputContainer         = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint          = code_point_t<_UEncoding>;
		constexpr bool _IsVoidContainer = ::std::is_void_v<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || is_unicode_code_point_v<_OutputCodePoint>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			// prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
			using _RealOutputContainer = ::std::basic_string<_OutputCodePoint>;
			return __txt_detail::__decode_dispatch<true, false, _RealOutputContainer>(
				::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else {
			using _RealOutputContainer
				= ::std::conditional_t<_IsVoidContainer, ::std::vector<_OutputCodePoint>, _OutputContainer>;
			return __txt_detail::__decode_dispatch<true, false, _RealOutputContainer>(
				::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding),
				::std::forward<_ErrorHandler>(__error_handler), __state);
		}
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	/// @param[in] __error_handler The error handlers for the from and to encodings,
	/// respectively.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function creates a `state` using ztd::text::make_decode_state.
	template <typename _OutputContainer = void, typename _Input, typename _Encoding, typename _ErrorHandler>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding, _ErrorHandler&& __error_handler) {
		using _UEncoding        = remove_cvref_t<_Encoding>;
		using _State            = decode_state_t<_UEncoding>;
		using _UOutputContainer = remove_cvref_t<_OutputContainer>;
		using _OutputCodePoint  = code_point_t<_UEncoding>;

		_State __state                  = ::ztd::text::make_decode_state(__encoding);
		constexpr bool _IsVoidContainer = ::std::is_void_v<_UOutputContainer>;
		constexpr bool _IsStringable
			= (is_char_traitable_v<_OutputCodePoint> || is_unicode_code_point_v<_OutputCodePoint>);
		if constexpr (_IsVoidContainer && _IsStringable) {
			// prevent instantiation errors with basic_string by boxing it inside of an "if constexpr"
			using _RealOutputContainer = ::std::basic_string<_OutputCodePoint>;
			return __txt_detail::__decode_dispatch<true, true, _RealOutputContainer>(::std::forward<_Input>(__input),
				::std::forward<_Encoding>(__encoding), ::std::forward<_ErrorHandler>(__error_handler), __state);
		}
		else {
			using _RealOutputContainer
				= ::std::conditional_t<_IsVoidContainer, ::std::vector<_OutputCodePoint>, _OutputContainer>;
			return __txt_detail::__decode_dispatch<true, true, _RealOutputContainer>(::std::forward<_Input>(__input),
				::std::forward<_Encoding>(__encoding), ::std::forward<_ErrorHandler>(__error_handler), __state);
		}
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	/// @param[in] __encoding The encoding that will be used to decode the input's code points into
	/// output code units.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function creates a `handler` using ztd::text::default_handler_t, but marks it as careless.
	template <typename _OutputContainer = void, typename _Input, typename _Encoding>
	constexpr auto decode(_Input&& __input, _Encoding&& __encoding) {
		default_handler_t __handler {};
		return ::ztd::text::decode<_OutputContainer>(
			::std::forward<_Input>(__input), ::std::forward<_Encoding>(__encoding), __handler);
	}

	//////
	/// @brief Converts the code units of the given `__input` view through the encoding to code points the
	/// specified
	/// `_OutputContainer` type.
	///
	/// @tparam _OutputContainer The container type to serialize data into.
	///
	/// @param[in] __input An input_view to read code units from and use in the decode operation that will
	/// produce code points.
	///
	/// @result An object of type `_OutputContainer` .
	///
	/// @remarks This function creates an `encoding` by using the `value_type` of the `__input` which is then
	/// passed through the ztd::text::default_code_point_encoding type to get the default desired encoding.
	template <typename _OutputContainer = void, typename _Input>
	constexpr auto decode(_Input&& __input) {
		using _UInput   = remove_cvref_t<_Input>;
		using _CodeUnit = ranges::range_value_type_t<_UInput>;
#if ZTD_IS_ON(ZTD_STD_LIBRARY_IS_CONSTANT_EVALUATED)
		if (::std::is_constant_evaluated()) {
			// Use literal encoding instead, if we meet the right criteria
			using _Encoding = default_consteval_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode<_OutputContainer>(::std::forward<_Input>(__input), __encoding);
		}
		else
#endif
		{
			using _Encoding = default_code_unit_encoding_t<_CodeUnit>;
			_Encoding __encoding {};
			return ::ztd::text::decode<_OutputContainer>(::std::forward<_Input>(__input), __encoding);
		}
	}

	//////
	/// @}

	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text

#include <ztd/epilogue.hpp>

#endif
