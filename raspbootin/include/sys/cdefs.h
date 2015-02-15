/* Copyright (C) 1992-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

// Striped down for MOOSE

#ifndef	_SYS_CDEFS_H
#define	_SYS_CDEFS_H	1

/* All functions, except those with callbacks or those that
   synchronize memory, are leaf functions.  */
#define __LEAF , __leaf__
#define __LEAF_ATTR __attribute__ ((__leaf__))

/* GCC can always grok prototypes.  For C++ programs we add throw()
   to help it optimize the function calls.  But this works only with
   gcc 2.8.x and egcs.  For gcc 3.2 and up we even mark C functions
   as non-throwing using a function attribute since programs can use
   the -fexceptions options for C code as well.  */
#define __THROW		__attribute__ ((__nothrow__ __LEAF))
#define __THROWNL	__attribute__ ((__nothrow__))
#define __NTH(fct)	__attribute__ ((__nothrow__ __LEAF)) fct

/* For these things, GCC behaves the ANSI way normally,
   and the non-ANSI way under -traditional.  */

#define __CONCAT(x,y)	x ## y
#define __STRING(x)	#x

/* C++ needs to know that types and declarations are C, not C++.  */
#ifdef	__cplusplus
# define __BEGIN_DECLS	extern "C" {
# define __END_DECLS	}
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif


/* The standard library needs the functions from the ISO C90 standard
   in the std namespace.  At the same time we want to be safe for
   future changes and we include the ISO C99 code in the non-standard
   namespace __c99.  The C++ wrapper header take case of adding the
   definitions to the global namespace.  */
#if defined __cplusplus && defined _GLIBCPP_USE_NAMESPACES
# define __BEGIN_NAMESPACE_STD	namespace std {
# define __END_NAMESPACE_STD	}
# define __USING_NAMESPACE_STD(name) using std::name;
# define __BEGIN_NAMESPACE_C99	namespace __c99 {
# define __END_NAMESPACE_C99	}
# define __USING_NAMESPACE_C99(name) using __c99::name;
# define __BEGIN_NAMESPACE(name)	namespace name {
# define __END_NAMESPACE(name)	}
# define __USING_NAMESPACE(name) using name;
#else
/* For compatibility we do not add the declarations into any
   namespace.  They will end up in the global namespace which is what
   old code expects.  */
# define __BEGIN_NAMESPACE_STD
# define __END_NAMESPACE_STD
# define __USING_NAMESPACE_STD(name)
# define __BEGIN_NAMESPACE_C99
# define __END_NAMESPACE_C99
# define __USING_NAMESPACE_C99(name)
# define __BEGIN_NAMESPACE(nam)
# define __END_NAMESPACE(name)
# define __USING_NAMESPACE(name)
#endif


#define __unlikely(cond)	__builtin_expect ((cond), 0)
#define __likely(cond)		__builtin_expect ((cond), 1)

/* Helpers for __attribute__ */
# define __nonnull(params) __attribute__ ((__nonnull__ params))
# define __attribute_warn_unused_result__ __attribute__ ((__warn_unused_result__))
# define __attribute_pure__ __attribute__ ((__pure__))

#endif	 /* sys/cdefs.h */
