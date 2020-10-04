// scope.h - Declarative Control Flow
/* Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Design from
https://www.youtube.com/watch?v=WjTrfoiB0MQ
Declarative Control Flow
Andrei Alexandrescu
*/

#pragma once

#include <utility>
#include <exception>

#ifndef CONCATENATE_IMPL
#define CONCATENATE_IMPL(s1, s2) s1##s2
#endif
#ifndef CONCATENATE
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)
#endif

#ifndef ANONYMOUS_VARIABLE
#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(str) CONCATENATE(str, __LINE__)
#endif
#endif

namespace Scope {
  template <typename FunctionType>
  class ScopeGuard {
    FunctionType function_;
  public:
    explicit ScopeGuard(const FunctionType& fn) : function_(fn) { }
    explicit ScopeGuard(const FunctionType&& fn) : function_(std::move(fn)) { }
    ~ScopeGuard() noexcept {
      function_();
    }
  };

  enum class ScopeGuardOnExit { };

  template <typename Fun>
  ScopeGuard<Fun> operator +(ScopeGuardOnExit, Fun&& fn) {
    return ScopeGuard<Fun>(std::forward<Fun>(fn));
  }

  class UncaughtExceptionCounter {
    int getUncaughtExecptionCount() noexcept;
    int exceptionCount_;
  public:
    UncaughtExceptionCounter() : exceptionCount_(std::uncaught_exceptions()) { }
    bool isNewUncaughtException() noexcept {
      return std::uncaught_exceptions() > exceptionCount_;
    }
  };

  template <typename FunctionType, bool executeOnException>
  class ScopeGuardForNewException {
    FunctionType function_;
    UncaughtExceptionCounter ec_;
  public:
    explicit ScopeGuardForNewException(const FunctionType& fn) : function_(fn) { }
    explicit ScopeGuardForNewException(const FunctionType&& fn) : function_(std::move(fn)) { }
    ~ScopeGuardForNewException() noexcept(executeOnException) {
      if (executeOnException == ec_.isNewUncaughtException()) {
        function_();
      }
    }
  };

  enum class ScopeGuardOnFail { };

  template <typename FunctionType>
  ScopeGuardForNewException<
    typename std::decay<FunctionType>::type, true>
  operator+(ScopeGuardOnFail, FunctionType&& fn) {
    return
      ScopeGuardForNewException<
        typename std::decay<FunctionType>::type, true>(
      std::forward<FunctionType>(fn));
  }

  enum class ScopeGuardOnSuccess { };

  template <typename FunctionType>
  ScopeGuardForNewException<
    typename std::decay<FunctionType>::type, false>
  operator+(ScopeGuardOnSuccess, FunctionType&& fn) {
    return
      ScopeGuardForNewException<
        typename std::decay<FunctionType>::type, false>(
      std::forward<FunctionType>(fn));
  }
}

#define SCOPE_EXIT \
  auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE) \
  = ::Scope::ScopeGuardOnExit() + [&]()

#define SCOPE_FAIL \
  auto ANONYMOUS_VARIABLE(SCOPE_FAIL_STATE) \
  = ::Scope::ScopeGuardOnFail() + [&]() noexcept

#define SCOPE_SUCCESS \
  auto ANONYMOUS_VARIABLE(SCOPE_FAIL_STATE) \
  = ::Scope::ScopeGuardOnSuccess() + [&]() noexcept

/*
Example:
========

void copy_file_transact(const path& from, const path& to) {
  bf::path t = to.native() + ".deleteme";
  SCOPE_FAIL { ::remove(t.c_str()); };
  bf::copy_file(from, to);
  bf::rename(t, to);
}

int string2int(const string& s) {
  int r;
  SCOPE_SUCCESS {
    assert(int2string(r) == s);
  };
  ...
  return r;
}
*/
