#pragma once
/*
 * Copyright (C) 2012 William Swanson
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the names of the authors or
 * their institutions shall not be used in advertising or otherwise to
 * promote the sale, use or other dealings in this Software without
 * prior written authorization from the authors.
 */

#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL5(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#ifdef _MSC_VER
// MSVC needs more evaluations
#define EVAL6(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL(...) EVAL6(EVAL6(__VA_ARGS__))
#else
#define EVAL(...) EVAL5(__VA_ARGS__)
#endif

#define MAP_END(...)
#define MAP_OUT

#define EMPTY()
#define DEFER(id) id EMPTY()

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) DEFER(MAP_NEXT0)(test, next, 0)
#define MAP_NEXT(test, next) MAP_NEXT1(MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) DEFER(MAP_NEXT(peek, MAP1))(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) DEFER(MAP_NEXT(peek, MAP0))(f, peek, __VA_ARGS__)

#define MAP_LIST0(f, x, peek, ...) \
  , f(x) DEFER(MAP_NEXT(peek, MAP_LIST1))(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) \
  , f(x) DEFER(MAP_NEXT(peek, MAP_LIST0))(f, peek, __VA_ARGS__)
#define MAP_LIST2(f, x, peek, ...) \
  f(x) DEFER(MAP_NEXT(peek, MAP_LIST1))(f, peek, __VA_ARGS__)

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

/**
 * Applies the function macro `f` to each of the remaining parameters and
 * inserts commas between the results.
 */
#define MAP_LIST(f, ...) EVAL(MAP_LIST2(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define MAP_LIST_MACRO(Cond) MAP_LIST_MACRO##Cond

#define MAP_LIST_COND(Cond, f, ...) MAP_LIST_MACRO(Cond)(f, __VA_ARGS__)

#define MAP_LIST_MACROtrue(f, ...) MAP_LIST(f, __VA_ARGS__)
#define MAP_LIST_MACROfalse(f, ...)

#define COND_COMMA(cond) COND_COMMA##cond
#define COND_COMMAtrue ,
#define COND_COMMAfalse

#endif

// Macros to remove parentheses from typenames when parens are present.
// https://stackoverflow.com/questions/24481810/how-to-remove-the-enclosing-parentheses-with-macro
#define DEPAREN(X) ESC(ISH X)
#define ISH(...) ISH __VA_ARGS__
#define ESC(...) ESC_(__VA_ARGS__)
#define ESC_(...) VAN##__VA_ARGS__
#define VANISH