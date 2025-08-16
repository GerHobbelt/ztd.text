.. =============================================================================
..
.. ztd.text
.. Copyright © JeanHeyd "ThePhD" Meneide and Shepherd's Oasis, LLC
.. Contact: opensource@soasis.org
..
.. Commercial License Usage
.. Licensees holding valid commercial ztd.text licenses may use this file in
.. accordance with the commercial license agreement provided with the
.. Software or, alternatively, in accordance with the terms contained in
.. a written agreement between you and Shepherd's Oasis, LLC.
.. For licensing terms and conditions see your agreement. For
.. further information contact opensource@soasis.org.
..
.. Apache License Version 2 Usage
.. Alternatively, this file may be used under the terms of Apache License
.. Version 2.0 (the "License") for non-commercial use; you may not use this
.. file except in compliance with the License. You may obtain a copy of the
.. License at
..
.. https://www.apache.org/licenses/LICENSE-2.0
..
.. Unless required by applicable law or agreed to in writing, software
.. distributed under the License is distributed on an "AS IS" BASIS,
.. WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
.. See the License for the specific language governing permissions and
.. limitations under the License.
..
.. =============================================================================>

``any_encoding`` omni-encoding type
===================================

``any_encoding`` is a class type whose sole purpose is to provide a type-generic, byte-based, runtime-deferred way of handling encodings.



Aliases
-------

.. doxygentypedef:: ztd::text::any_encoding

.. doxygentypedef:: ztd::text::any_encoding_of

.. doxygentypedef:: ztd::text::compat_any_encoding

.. doxygentypedef:: ztd::text::ucompat_any_encoding



Base
----

This is based off of :cpp:class:`ztd::text::any_encoding_with`; the only thing special about it is the constructor logic on top of :cpp:class:`ztd::text::any_encoding_with`.

.. doxygenclass:: ztd::text::any_byte_encoding
	:members: any_byte_encoding, operator=
	
