/*
 * Copyright (C) 2016-2017 Max Kellermann <max.kellermann@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EXCEPTION_HXX
#define EXCEPTION_HXX

#include "Compiler.h"

#include <exception>
#include <string>

/**
 * Throws the specified exception.  There is an overload for
 * std::exception_ptr which throws the contained exception instead of
 * the std::exception_ptr itself.
 */
template<typename T>
gcc_noreturn
inline void
ThrowException(T &&t)
{
	throw t;
}

gcc_noreturn
inline void
ThrowException(std::exception_ptr ep)
{
	std::rethrow_exception(ep);
}

/**
 * Create a nested exception, wrapping #ep inside (a copy of) #t.
 */
template<typename T>
inline std::exception_ptr
NestException(std::exception_ptr ep, T &&t) noexcept
{
	try {
		std::rethrow_exception(ep);
	} catch (...) {
		try {
			std::throw_with_nested(std::forward<T>(t));
		} catch (...) {
			return std::current_exception();
		}
	}
}

/**
 * Find an instance of #T in the nested exception chain, and rethrow
 * it.  Does nothing of no such instance was found.
 */
template<typename T>
inline void
FindRetrowNested(std::exception_ptr ep)
{
	try {
		std::rethrow_exception(ep);
	} catch (const T &t) {
		throw;
	} catch (const std::exception &e) {
		try {
			std::rethrow_if_nested(e);
		} catch (...) {
			FindRetrowNested<T>(std::current_exception());
		}
	} catch (const std::nested_exception &ne) {
		FindRetrowNested<T>(ne.nested_ptr());
	} catch (...) {
	}
}

/**
 * Obtain the full concatenated message of an exception and its nested
 * chain.
 */
std::string
GetFullMessage(const std::exception &e,
	       const char *fallback="Unknown exception",
	       const char *separator="; ") noexcept;

/**
 * Extract the full message of a C++ exception, considering its nested
 * exceptions (if any).
 */
std::string
GetFullMessage(std::exception_ptr ep,
	       const char *fallback="Unknown exception",
	       const char *separator="; ") noexcept;

#endif
