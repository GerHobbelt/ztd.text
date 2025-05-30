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

#ifndef ZTD_TEXT_BASIC_ICONV_HPP
#define ZTD_TEXT_BASIC_ICONV_HPP

#include <ztd/text/version.hpp>

#include <ztd/text/detail/basic_iconv_includes.hpp>

#include <ztd/prologue.hpp>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	namespace __txt_detail {
		template <typename _CodeUnit, typename _CodePoint>
		using __basic_iconv_base =
#if ZTD_IS_ON(ZTD_PLATFORM_LIBICONV)
			__empty_state
#else
			basic_no_encoding<_CodeUnit, _CodePoint>
#endif
			;
	} // namespace __txt_detail

	//////
	/// @brief An encoding which is templated on code unit and code point and provides access to the `iconv` library.
	///
	/// @tparam _CodeUnit The code unit type.
	/// @tparam _CodePoint The code point type.
	///
	/// @remarks The type is created with a name and provides a conversion routine when used. Because it is all done
	/// at runtime, it is considered a lossy conversion and thus requires prolific use of error handlers. If libiconv
	/// cannot be found, this type will produce a hard error on use.
	template <typename _CodeUnit, typename _CodePoint = unicode_code_point>
	class basic_iconv : public __txt_detail::__basic_iconv_base<_CodeUnit, _CodePoint> {
#if ZTD_IS_ON(ZTD_PLATFORM_LIBICONV)
	private:
		static_assert(always_true_v<_CodeUnit> && ZTD_IS_ON(ZTD_PLATFORM_LIBICONV),
			"Cannot use the iconv encoding without either the headers or a dynamic runtime loader being "
			"present!");

		inline static constexpr ::std::size_t _MaxDrainSize = 64;

		void _M_reset_state(::ztd::plat::icnv::descriptor __desc) const noexcept {
			const auto& __iconv_functions = ::ztd::plat::icnv::functions();
			char __drain[_MaxDrainSize];
			char* __p_drain                          = __drain;
			const ::std::size_t __initial_drain_size = sizeof(__drain);
			::std::size_t __drain_size               = __initial_drain_size;
			const ::std::size_t __suggested_clear_result
				= __iconv_functions.convert(__desc, nullptr, nullptr, &__p_drain, &__drain_size);
			ZTD_TEXT_ASSERT(__suggested_clear_result == ::ztd::plat::icnv::conversion_success);
			if (__initial_drain_size != __drain_size) {
				// there is a special sequence that will reset the conversion back to normal
				char __faux_drain[_MaxDrainSize];
				char* __p_faux_drain                          = __faux_drain;
				const ::std::size_t __initial_faux_drain_size = sizeof(__faux_drain);
				::std::size_t __faux_drain_size               = __initial_faux_drain_size;
				const char* __p_input                         = __drain;
				const ::std::size_t __initial_input_size      = __initial_drain_size - __drain_size;
				::std::size_t __input_size                    = __initial_input_size;
				const ::std::size_t __clear_result            = __iconv_functions.convert(
                         __desc, &__p_input, &__input_size, &__p_faux_drain, &__faux_drain_size);
				ZTD_TEXT_ASSERT(__clear_result == ::ztd::plat::icnv::conversion_success);
				ZTD_TEXT_ASSERT(__input_size == 0);
			}
			else {
				// there is no special sequence: just erase it normally
				const ::std::size_t __clear_result
					= __iconv_functions.convert(__desc, nullptr, nullptr, nullptr, nullptr);
				ZTD_TEXT_ASSERT(__clear_result == ::ztd::plat::icnv::conversion_success);
			}
		}

		bool _M_destroy_bom(c_string_view __to_name, ::ztd::plat::icnv::descriptor __desc,
			::std::size_t __from_size) const noexcept {
			this->_M_reset_state(__desc);
			constexpr ::std::size_t __max_input_size = 5;
			// Are we going to a Unicode encoding?
			if (!::ztd::is_unicode_encoding_name(__to_name.base()) || __from_size > __max_input_size) {
				return true;
			}
			// Congratulations, welcome to the hell that is the GNU / libc implementation of iconv!
			// Instead of creating special encodings to handle byte order marks, or using iconvctl(...) to
			// enable it specifically in the case of the iconv utility, they instead poisoned most/all
			// Unicode encodings by forcing them to write a byte order mark at the start. Very cool and
			// normal and good of them to do this, so we need to poison the water here and manually force
			// the vomiting-out of a single byte order mark by any means necessary.
			//
			// Cue the usual "EXCUSE ME, BUT THIS IS A UTILTIY USED FOR UNIX, AND UNIX TOOLS AND NEEDS TAKE
			// PRIORITY!!!!"
			//
			// Unfortunately, serving one need doesn't mean slamming a spiked bat into everyone else's gut.
			//
			// And everyone wonders why they'd rather go use something else OTHER than libiconv...
			constexpr const char __inputs[][__max_input_size] = {
				// 1-sized
				{ 0x0A, 0x00, 0x00, 0x00, 0x00 },
				{ 0x20, 0x00, 0x00, 0x00, 0x00 },
				{ 0x7F, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x00, 0x00, 0x00, 0x00 },
				// 2-sized
				{ 0x0A, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x0A, 0x00, 0x00, 0x00 },
				{ 0x20, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x20, 0x00, 0x00, 0x00 },
				{ 0x7F, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x7F, 0x00, 0x00, 0x00 },
				{ 0x00, 0x00, 0x00, 0x00, 0x00 },
				// 4-sized
				{ 0x0A, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x00, 0x00, 0x0A, 0x00 },
				{ 0x7F, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x00, 0x00, 0x7F, 0x00 },
				{ 0x20, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x00, 0x00, 0x20, 0x00 },
				{ 0x00, 0x00, 0x00, 0x00, 0x00 },
			};
			constexpr const ::std::size_t __inputs_size = ::std::size(__inputs);
			char __drain[_MaxDrainSize];
			const ::std::size_t __preferred_index_start = __from_size <= 1 ? 0 : (__from_size <= 2 ? 4 : 11);
			for (::std::size_t __input_index = __preferred_index_start; __input_index < __inputs_size;
				++__input_index) {
				auto& __input              = __inputs[__input_index];
				char* __p_drain            = __drain;
				::std::size_t __drain_size = sizeof(__drain);
				char __input_write_bom[__max_input_size];
				ranges::__rng_detail::__copy_n_unsafe(__input + 0, __max_input_size, __input_write_bom + 0);
				const char* __p_input_write_bom        = __input_write_bom;
				::std::size_t __input_write_bom_size   = __from_size;
				const ::std::size_t __write_bom_result = ::ztd::plat::icnv::functions().convert(
					__desc, &__p_input_write_bom, &__input_write_bom_size, &__p_drain, &__drain_size);
				if (__write_bom_result == ::ztd::plat::icnv::conversion_success) {
					// we found something! Thank god...
					return true;
				}
				this->_M_reset_state(__desc);
			}

			// if we get here and we've never found success, well, it's time to just go home and cry!
			// rot in hell, POSIX.
			return false;
		}

		void _M_reset_descriptor(c_string_view __to_name, ::ztd::plat::icnv::descriptor __desc,
			::std::size_t __from_size) const noexcept {
			if (!this->_M_destroy_bom(__to_name, __desc, __from_size)) {
				// clear things again, just to be safe
				this->_M_reset_state(__desc);
			}
		}

	public:
		//////
		/// @brief The state for decode operations.
		///
		/// @remarks This contains the actual conversion descriptor for iconv. When the state disappears, so does
		/// its descriptor.
		struct decode_state {
			::ztd::plat::icnv::descriptor _M_conv_descriptor;

			decode_state(const basic_iconv& __source) noexcept
			: _M_conv_descriptor(::ztd::plat::icnv::failure_descriptor) {
				this->_M_conv_descriptor = ::ztd::plat::icnv::functions().open(
					__source._M_to_name.c_str(), __source._M_from_name.c_str());
				if (this->_M_is_valid()) {
					__source._M_reset_descriptor(__source._M_to_name, this->_M_conv_descriptor, sizeof(_CodeUnit));
				}
			}

			bool _M_is_valid() const noexcept {
				return ::ztd::plat::icnv::descriptor_is_valid(this->_M_conv_descriptor);
			}

			~decode_state() {
				if (this->_M_conv_descriptor != ::ztd::plat::icnv::failure_descriptor) {
					int __close_result = ::ztd::plat::icnv::functions().close(this->_M_conv_descriptor);
					ZTD_TEXT_ASSERT(__close_result == ::ztd::plat::icnv::close_success);
				}
			}
		};

		//////
		/// @brief The state for encode operations.
		///
		/// @remarks This contains the ac        tual conversion descriptor for iconv. When the state disappears,
		/// so does its descriptor.
		struct encode_state {
			::ztd::plat::icnv::descriptor _M_conv_descriptor;

			encode_state(const basic_iconv& __source) noexcept
			: _M_conv_descriptor(::ztd::plat::icnv::failure_descriptor) {
				this->_M_conv_descriptor = ::ztd::plat::icnv::functions().open(
					__source._M_from_name.c_str(), __source._M_to_name.c_str());
				if (this->_M_is_valid()) {
					__source._M_reset_descriptor(
						__source._M_from_name, this->_M_conv_descriptor, sizeof(_CodePoint));
				}
			}

			bool _M_is_valid() const noexcept {
				return ::ztd::plat::icnv::descriptor_is_valid(this->_M_conv_descriptor);
			}

			~encode_state() {
				if (this->_M_conv_descriptor != ::ztd::plat::icnv::failure_descriptor) {
					int __close_result = ::ztd::plat::icnv::functions().close(this->_M_conv_descriptor);
					ZTD_TEXT_ASSERT(__close_result == ::ztd::plat::icnv::close_success);
				}
			}
		};

		//////
		/// @brief The code unit type used for input on decode operations and output for encode operations.
		using code_unit = _CodeUnit;
		//////
		/// @brief The code point type used for output on decode operations and input for encode operations.
		using code_point = _CodePoint;

		//////
		/// @brief The maximum number of code units that can be output by a single operation.
		///
		/// @remarks Since this is a runtime-based encoding, these numbers are set abnormally high, in hopes that
		/// they never need to be changed.
		inline static constexpr ::std::size_t max_code_units = 32;
		//////
		/// @brief The maximum number of code points that can be output by a single operation.
		///
		/// @remarks Since this is a runtime-based encoding, these numbers are set abnormally high, in hopes that
		/// they never need to be changed.
		inline static constexpr ::std::size_t max_code_points = 32;


		bool contains_unicode_encoding() const noexcept {
			return ::ztd::is_unicode_encoding_name(this->_M_from_name)
				&& ::ztd::is_unicode_encoding_name(this->_M_to_name);
		}

		//////
		/// @brief Decodes a single complete unit of information as code points and produces a result with the
		/// input and output ranges moved past what was successfully read and written; or, produces an error and
		/// returns the input and output ranges untouched.
		///
		/// @param[in] __input The input view to read code uunits from.
		/// @param[in] __output The output view to write code points into.
		/// @param[in] __error_handler The error handler to invoke if encoding fails.
		/// @param[in, out] __state The necessary state information. For this encoding, the state is empty and
		/// means very little.
		///
		/// @returns A ztd::text::decode_result object that contains the input range, output range, error handler, and
		/// a reference to the passed-in state\.
		///
		/// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
		/// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
		/// incremented even if an error occurs due to the semantics of any view that models an input_range.
		template <typename _Input, typename _Output, typename _ErrorHandler>
		auto decode_one(_Input&& __input, _Output&& __output, _ErrorHandler&& __error_handler,
			decode_state& __state) const noexcept {
			using _UErrorHandler                = remove_cvref_t<_ErrorHandler>;
			using _SubInput                     = ztd::ranges::csubrange_for_t<::std::remove_reference_t<_Input>>;
			using _SubOutput                    = ztd::ranges::subrange_for_t<::std::remove_reference_t<_Output>>;
			using _Result                       = decode_result<_SubInput, _SubOutput, decode_state>;
			constexpr bool __call_error_handler = !is_ignorable_error_handler_v<_UErrorHandler>;

			if constexpr (__call_error_handler) {
				if (!__state._M_is_valid()) {
					// bail instead of destroying everything
					return ::std::forward<_ErrorHandler>(__error_handler)(*this,
						_Result(::std::forward<_Input>(__input), ::std::forward<_Output>(__output), __state,
						     encoding_error::invalid_sequence),
						::ztd::span<const code_unit, 0>(), ::ztd::span<const code_point, 0>());
				}
			}

			auto __in_it   = ::ztd::ranges::cbegin(__input);
			auto __in_last = ::ztd::ranges::cend(__input);

			if (__in_it == __in_last) {
				// an exhausted sequence is fine
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					::std::forward<_Output>(__output), __state, encoding_error::ok);
			}

			auto __out_it   = ::ztd::ranges::begin(__output);
			auto __out_last = ::ztd::ranges::end(__output);

			code_unit __read_buffer[max_code_units];
			code_point __write_buffer[max_code_points];
			constexpr ::std::size_t __maximum_read_size         = std::size(__read_buffer);
			constexpr ::std::size_t __initial_write_size        = std::size(__write_buffer);
			constexpr ::std::size_t __initial_write_buffer_size = __initial_write_size * sizeof(__write_buffer[0]);
			auto* __write_pointer                               = reinterpret_cast<char*>(__write_buffer + 0);
			::std::size_t __write_buffer_size                   = __initial_write_buffer_size;
			std::size_t __read_index                            = 0;
			for (; __read_index < __maximum_read_size; ++__read_index) {
				__read_buffer[__read_index] = *__in_it;
				++__in_it;
				::std::size_t __read_buffer_size = (__read_index + 1) * sizeof(__read_buffer[0]);
				auto* __read_pointer             = reinterpret_cast<char*>(::std::addressof(__read_buffer[0]));
				::std::size_t __attempted_write_result = ::ztd::plat::icnv::functions().convert(
					__state._M_conv_descriptor, ::std::addressof(__read_pointer), &__read_buffer_size,
					::std::addressof(__write_pointer), &__write_buffer_size);
				if (__attempted_write_result == ::ztd::plat::icnv::conversion_failure) {
					ZTD_TEXT_ASSERT(errno != EBADF && errno != E2BIG);
					switch (errno) {
					case EINVAL:
						// this can be fine, if we have more to read
						if constexpr (__call_error_handler) {
							if (__in_it == __in_last) {
								return ::std::forward<_ErrorHandler>(__error_handler)(*this,
									_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
									     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
									     encoding_error::invalid_sequence),
									::ztd::span<const code_unit>(static_cast<code_unit*>(__read_buffer + 0),
									     static_cast<::std::size_t>(__read_index + 1)),
									::ztd::span<const code_point, 0>());
							}
						}
						// okay, loop back around!
						continue;
					case EILSEQ:
						if constexpr (__call_error_handler) {
							// bad input bytes: time to retreat...
							return ::std::forward<_ErrorHandler>(__error_handler)(*this,
								_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
								     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
								     encoding_error::invalid_sequence),
								::ztd::span<const code_unit>(static_cast<code_unit*>(__read_buffer + 0),
								     static_cast<::std::size_t>(__read_index + 1)),
								::ztd::span<const code_point, 0>());
						}
					default:
						break;
					}
				}
				::std::size_t __written_size
					= (__initial_write_buffer_size - __write_buffer_size) / sizeof(__write_buffer[0]);
				for (::std::size_t __write_index = 0; __write_index < __written_size; ++__write_index) {
					// drain into output since it's all fine
					if constexpr (__call_error_handler) {
						if (__out_it == __out_last) {
							// insufficient space!
							return ::std::forward<_ErrorHandler>(__error_handler)(*this,
								_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
								     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
								     encoding_error::insufficient_output_space),
								::ztd::span<const code_unit>(static_cast<code_unit*>(__read_buffer + 0),
								     static_cast<::std::size_t>(__read_index + 1)),
								::ztd::span<const code_point>(
								     static_cast<code_point*>(__write_buffer + __write_index),
								     static_cast<::std::size_t>(__written_size - __write_index)));
						}
					}
					*__out_it = static_cast<code_point>(__write_buffer[__write_index]);
					++__out_it;
				}
				// all... okay!
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					_SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state, encoding_error::ok);
			}
			if constexpr (__call_error_handler) {
				return ::std::forward<_ErrorHandler>(__error_handler)(*this,
					_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
					     encoding_error::invalid_sequence),
					::ztd::span<const code_unit>(
					     static_cast<code_unit*>(__read_buffer + 0), static_cast<::std::size_t>(__read_index + 1)),
					::ztd::span<const code_point, 0>());
			}
			else {
				// ... welllll, okay?!
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					_SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state, encoding_error::ok);
			}
		}

		//////
		/// @brief Encodes a single complete unit of information as code units and produces a result with the
		/// input and output ranges moved past what was successfully read and written; or, produces an error and
		/// returns the input and output ranges untouched.
		///
		/// @param[in] __input The input view to read code points from.
		/// @param[in] __output The output view to write code units into.
		/// @param[in] __error_handler The error handler to invoke if encoding fails.
		/// @param[in, out] __state The necessary state information. For this encoding, the state is empty and
		/// means very little.
		///
		/// @returns A ztd::text::encode_result object that contains the input range, output range, error handler, and
		/// a reference to the passed-in state\.
		///
		/// @remarks To the best ability of the implementation, the iterators will be returned untouched (e.g.,
		/// the input models at least a view and a forward_range). If it is not possible, returned ranges may be
		/// incremented even if an error occurs due to the semantics of any view that models an input_range.
		template <typename _Input, typename _Output, typename _ErrorHandler>
		auto encode_one(_Input&& __input, _Output&& __output, _ErrorHandler&& __error_handler,
			encode_state& __state) const noexcept {
			using _UErrorHandler                = remove_cvref_t<_ErrorHandler>;
			using _SubInput                     = ztd::ranges::csubrange_for_t<::std::remove_reference_t<_Input>>;
			using _SubOutput                    = ztd::ranges::subrange_for_t<::std::remove_reference_t<_Output>>;
			using _Result                       = encode_result<_SubInput, _SubOutput, encode_state>;
			constexpr bool __call_error_handler = !is_ignorable_error_handler_v<_UErrorHandler>;

			if constexpr (__call_error_handler) {
				if (!__state._M_is_valid()) {
					// bail instead of destroying everything
					return ::std::forward<_ErrorHandler>(__error_handler)(*this,
						_Result(::std::forward<_Input>(__input), ::std::forward<_Output>(__output), __state,
						     encoding_error::invalid_sequence),
						::ztd::span<const code_point, 0>(), ::ztd::span<const code_unit, 0>());
				}
			}

			auto __in_it   = ::ztd::ranges::cbegin(__input);
			auto __in_last = ::ztd::ranges::cend(__input);

			if (__in_it == __in_last) {
				// an exhausted sequence is fine
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					::std::forward<_Output>(__output), __state, encoding_error::ok);
			}

			auto __out_it   = ::ztd::ranges::begin(__output);
			auto __out_last = ::ztd::ranges::end(__output);

			code_point __read_buffer[max_code_points];
			code_unit __write_buffer[max_code_units];
			constexpr ::std::size_t __maximum_read_size         = std::size(__read_buffer);
			constexpr ::std::size_t __initial_write_size        = std::size(__write_buffer);
			constexpr ::std::size_t __initial_write_buffer_size = __initial_write_size * sizeof(__write_buffer[0]);
			auto* __write_pointer                               = reinterpret_cast<char*>(__write_buffer + 0);
			::std::size_t __write_buffer_size                   = __initial_write_buffer_size;
			std::size_t __read_index                            = 0;
			for (; __read_index < __maximum_read_size; ++__read_index) {
				__read_buffer[__read_index] = *__in_it;
				++__in_it;
				const ::std::size_t __initial_read_buffer_size = (__read_index + 1) * sizeof(__read_buffer[0]);
				::std::size_t __read_buffer_size               = __initial_read_buffer_size;
				auto* __read_pointer = reinterpret_cast<char*>(::std::addressof(__read_buffer[0]));
				::std::size_t __attempted_write_result = ::ztd::plat::icnv::functions().convert(
					__state._M_conv_descriptor, ::std::addressof(__read_pointer), &__read_buffer_size,
					::std::addressof(__write_pointer), &__write_buffer_size);
				if (__attempted_write_result == ::ztd::plat::icnv::conversion_failure) {
					ZTD_TEXT_ASSERT(errno != EBADF && errno != E2BIG);
					switch (errno) {
					case EINVAL:
						// this can be fine, if we have more to read
						if constexpr (__call_error_handler) {
							if (__in_it == __in_last) {
								return ::std::forward<_ErrorHandler>(__error_handler)(*this,
									_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
									     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
									     encoding_error::invalid_sequence),
									::ztd::span<const code_point>(static_cast<code_point*>(__read_buffer + 0),
									     static_cast<::std::size_t>(__read_index + 1)),
									::ztd::span<const code_unit, 0>());
							}
						}
						// okay, loop back around!
						continue;
					case EILSEQ:
						if constexpr (__call_error_handler) {
							// bad input bytes: time to retreat...
							return ::std::forward<_ErrorHandler>(__error_handler)(*this,
								_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
								     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
								     encoding_error::invalid_sequence),
								::ztd::span<const code_point>(static_cast<code_point*>(__read_buffer + 0),
								     static_cast<::std::size_t>(__read_index + 1)),
								::ztd::span<const code_unit, 0>());
						}
					default:
						break;
					}
				}
				::std::size_t __written_size
					= (__initial_write_buffer_size - __write_buffer_size) / sizeof(__write_buffer[0]);
				for (::std::size_t __write_index = 0; __write_index < __written_size; ++__write_index) {
					// drain into output since it's all fine
					if constexpr (__call_error_handler) {
						if (__out_it == __out_last) {
							// insufficient space!
							return ::std::forward<_ErrorHandler>(__error_handler)(*this,
								_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
								     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
								     encoding_error::insufficient_output_space),
								::ztd::span<const code_point>(static_cast<code_point*>(__read_buffer + 0),
								     static_cast<::std::size_t>(__read_index + 1)),
								::ztd::span<const code_unit>(
								     static_cast<code_unit*>(__write_buffer + __write_index),
								     static_cast<::std::size_t>(__written_size - __write_index)));
						}
					}
					*__out_it = static_cast<code_unit>(__write_buffer[__write_index]);
					++__out_it;
				}
				// all... okay!
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					_SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state, encoding_error::ok);
			}
			if constexpr (__call_error_handler) {
				return ::std::forward<_ErrorHandler>(__error_handler)(*this,
					_Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					     _SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state,
					     encoding_error::invalid_sequence),
					::ztd::span<const code_point>(static_cast<code_point*>(__read_buffer + 0),
					     static_cast<::std::size_t>(__read_index + 1)),
					::ztd::span<const code_unit, 0>());
			}
			else {
				// ... welllll, okay?!
				return _Result(_SubInput(::std::move(__in_it), ::std::move(__in_last)),
					_SubOutput(::std::move(__out_it), ::std::move(__out_last)), __state, encoding_error::ok);
			}
		}

#endif
	public:
		//////
		/// @brief Creates an iconv encoding that refers to both of the provided names.
		///
		/// @param[in] __from_name The name of the encoding in iconv to decode from.
		/// @param[in] __to_name The name of the encoding in iconv to encode to.
		basic_iconv(std::string_view __from_name,
			std::string_view __to_name = __txt_detail::__platform_utf_name<_CodePoint>().base())
		: _M_from_name(__from_name), _M_to_name(__to_name) {
		}

	private:
		std::string _M_from_name;
		std::string _M_to_name;
	};


	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text

#include <ztd/epilogue.hpp>

#endif
