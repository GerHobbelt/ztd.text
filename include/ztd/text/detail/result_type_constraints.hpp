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

#ifndef ZTD_TEXT_DETAIL_RESULT_TYPE_CONSTRAINTS_HPP
#define ZTD_TEXT_DETAIL_RESULT_TYPE_CONSTRAINTS_HPP

#include <ztd/text/version.hpp>

#include <ztd/idk/reference_wrapper.hpp>
#include <ztd/idk/type_traits.hpp>

#include <ztd/prologue.hpp>

namespace ztd { namespace text {
	ZTD_TEXT_INLINE_ABI_NAMESPACE_OPEN_I_

	namespace __txt_detail {

		template <template <typename...> class _ResultType, typename _Input, typename _ArgInput, typename _Output,
			typename _ArgOutput, typename _State = void, typename _ArgState = void, typename _State2 = void,
			typename _ArgState2 = void, typename _Pivot = void, typename _ArgPivot = void>
		constexpr bool __result_type_copy_noexcept() noexcept {
			if constexpr (::std::is_void_v<_State>) {
				return !::std::is_nothrow_constructible_v<_Input, const _ArgInput&>
					&& ::std::is_nothrow_constructible_v<_Output, const _ArgOutput&>;
			}
			else {
				if constexpr (::std::is_void_v<_State2>) {
					return ::std::is_nothrow_constructible_v<_Input, const _ArgInput&>
						&& ::std::is_nothrow_constructible_v<_Output, const _ArgOutput&>
						&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>;
				}
				else {
					if constexpr (::std::is_void_v<_Pivot>) {
						return ::std::is_nothrow_constructible_v<_Input, const _ArgInput&>
							&& ::std::is_nothrow_constructible_v<_Output, const _ArgOutput&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>;
					}
					else {
						return ::std::is_nothrow_constructible_v<_Input, const _ArgInput&>
							&& ::std::is_nothrow_constructible_v<_Output, const _ArgOutput&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>
							&& ::std::is_nothrow_constructible_v<_Pivot, const _ArgPivot&>;
					}
				}
			}
		}

		template <template <typename...> class _ResultType, typename _Input, typename _ArgInput, typename _Output,
			typename _ArgOutput, typename _State = void, typename _ArgState = void, typename _State2 = void,
			typename _ArgState2 = void, typename _Pivot = void, typename _ArgPivot = void>
		constexpr bool __result_type_move_noexcept() noexcept {
			if constexpr (::std::is_void_v<_State>) {
				return ::std::is_nothrow_constructible_v<_Input, _ArgInput&&>
					&& ::std::is_nothrow_constructible_v<_Output, _ArgOutput&&>;
			}
			else {
				if constexpr (::std::is_void_v<_State2>) {
					return ::std::is_nothrow_constructible_v<_Input, _ArgInput&&>
						&& ::std::is_nothrow_constructible_v<_Output, _ArgOutput&&>
						&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>;
				}
				else {
					if constexpr (::std::is_void_v<_Pivot>) {
						return ::std::is_nothrow_constructible_v<_Input, _ArgInput&&>
							&& ::std::is_nothrow_constructible_v<_Output, _ArgOutput&&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>;
					}
					else {
						return ::std::is_nothrow_constructible_v<_Input, _ArgInput&&>
							&& ::std::is_nothrow_constructible_v<_Output, _ArgOutput&&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_nothrow_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>
							&& ::std::is_nothrow_constructible_v<_Pivot, _ArgPivot&&>;
					}
				}
			}
		}

		template <template <typename...> class _ResultType, typename _Input, typename _ArgInput, typename _Output,
			typename _ArgOutput, typename _State = void, typename _ArgState = void, typename _State2 = void,
			typename _ArgState2 = void, typename _Pivot = void, typename _ArgPivot = void>
		constexpr bool __result_type_copy_constraint() noexcept {
			if constexpr (::std::is_void_v<_State>) {
				return !::std::is_same_v<_ResultType<_Input, _Output>, _ResultType<_ArgInput, _ArgOutput>>
					&& ::std::is_constructible_v<_Input, const _ArgInput&>
					&& ::std::is_constructible_v<_Output, const _ArgOutput&>;
			}
			else {
				if constexpr (::std::is_void_v<_State2>) {
					return !::std::is_same_v<_ResultType<_Input, _Output, _State>,
						       _ResultType<_ArgInput, _ArgOutput, _ArgState>>
						&& ::std::is_constructible_v<_Input, const _ArgInput&>
						&& ::std::is_constructible_v<_Output, const _ArgOutput&>
						&& ::std::is_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>;
				}
				else {
					if constexpr (::std::is_void_v<_Pivot>) {
						return !::std::is_same_v<_ResultType<_Input, _Output, _State, _State2>,
							       _ResultType<_ArgInput, _ArgOutput, _ArgState, _ArgState2>>
							&& ::std::is_constructible_v<_Input, const _ArgInput&>
							&& ::std::is_constructible_v<_Output, const _ArgOutput&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>;
					}
					else {
						return !::std::is_same_v<_ResultType<_Input, _Output, _State, _State2, _Pivot>,
							       _ResultType<_ArgInput, _ArgOutput, _ArgState, _ArgState2, _ArgPivot>>
							&& ::std::is_constructible_v<_Input, const _ArgInput&>
							&& ::std::is_constructible_v<_Output, const _ArgOutput&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>
							&& ::std::is_constructible_v<_Pivot, const _ArgPivot&>;
					}
				}
			}
		}

		template <template <typename...> class _ResultType, typename _Input, typename _ArgInput, typename _Output,
			typename _ArgOutput, typename _State = void, typename _ArgState = void, typename _State2 = void,
			typename _ArgState2 = void, typename _Pivot = void, typename _ArgPivot = void>
		constexpr bool __result_type_move_constraint() noexcept {
			if constexpr (::std::is_void_v<_State>) {
				return !::std::is_same_v<_ResultType<_Input, _Output>, _ResultType<_ArgInput, _ArgOutput>>
					&& ::std::is_constructible_v<_Input, _ArgInput&&>
					&& ::std::is_constructible_v<_Output, _ArgOutput&&>;
			}
			else {
				if constexpr (::std::is_void_v<_State2>) {
					return !::std::is_same_v<_ResultType<_Input, _Output, _State>,
						       _ResultType<_ArgInput, _ArgOutput, _ArgState>>
						&& ::std::is_constructible_v<_Input, _ArgInput&&>
						&& ::std::is_constructible_v<_Output, _ArgOutput&&>
						&& ::std::is_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>;
				}
				else {
					if constexpr (::std::is_void_v<_Pivot>) {
						return !::std::is_same_v<_ResultType<_Input, _Output, _State, _State2>,
							       _ResultType<_ArgInput, _ArgOutput, _ArgState, _ArgState2>>
							&& ::std::is_constructible_v<_Input, _ArgInput&&>
							&& ::std::is_constructible_v<_Output, _ArgOutput&&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>;
					}
					else {
						return !::std::is_same_v<_ResultType<_Input, _Output, _State, _State2, _Pivot>,
							       _ResultType<_ArgInput, _ArgOutput, _ArgState, _ArgState2, _ArgPivot>>
							&& ::std::is_constructible_v<_Input, _ArgInput&&>
							&& ::std::is_constructible_v<_Output, _ArgOutput&&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State>, _ArgState&>
							&& ::std::is_constructible_v<::ztd::reference_wrapper<_State2>, _ArgState2&>
							&& ::std::is_constructible_v<_Pivot, _ArgPivot&&>;
					}
				}
			}
		}
	} // namespace __txt_detail

	ZTD_TEXT_INLINE_ABI_NAMESPACE_CLOSE_I_
}} // namespace ztd::text

#include <ztd/epilogue.hpp>

#endif
