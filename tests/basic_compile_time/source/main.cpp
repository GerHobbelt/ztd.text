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

#include <ztd/text.hpp>

#include <iostream>

int main(void) {
	std::cout << "=== Encoding Names ===" << std::endl;
	std::cout << "Literal Encoding: "
	          << ztd::__idk_detail::to_name(
	                  ztd::__idk_detail::__to_encoding_id(ZTD_CXX_COMPILE_TIME_ENCODING_NAME_GET_I_()))
	          << std::endl;
	std::cout << "Wide Literal Encoding: "
	          << ztd::__idk_detail::to_name(
	                  ztd::__idk_detail::__to_encoding_id(ZTD_CXX_COMPILE_TIME_WIDE_ENCODING_NAME_GET_I_()))
	          << std::endl;
	return 0;
}
