/* //-*- C++ -*-///////////////////////////////////////////////////////////// *\

  Copyright 2024
  Miguel Young de la Sota and the Best Contributors 🧶🐈‍⬛

  Licensed under the Apache License, Version 2.0 (the "License"); you may not
  use this file except in compliance with the License. You may obtain a copy
  of the License at

                https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
  License for the specific language governing permissions and limitations
  under the License.

\* ////////////////////////////////////////////////////////////////////////// */

#ifndef BEST_META_OPS_H_
#define BEST_META_OPS_H_

#include <stddef.h>

#include "best/meta/init.h"
#include "best/meta/internal/ops.h"
#include "best/meta/traits.h"

//! Helpers for working with overloadable operators.

namespace best {
/// An overloadable operator.
enum class op {
  Add,   // a + b
  Sub,   // a - b
  Mul,   // a * b
  Div,   // a / b
  Rem,   // a % b
  Plus,  // +a
  Neg,   // -a

  AndAnd,  // a && b
  OrOr,    // a || b
  Not,     // !a

  And,   // a & b
  Or,    // a | b
  Xor,   // a ^ b
  Shl,   // a << b
  Shr,   // a >> b
  Cmpl,  // ~a

  Deref,      // *a
  AddrOf,     // &a
  Arrow,      // a->m
  ArrowStar,  // a->*m

  Eq,  // a == b
  Ne,  // a != b
  Lt,  // a < b
  Le,  // a <= b
  Gt,  // a > b
  Ge,  // a >= b

  Spaceship,  // a <=> b
  Comma,      // a, b

  Call,   // a(b)
  Index,  // a[b]

  Assign,     // a = b
  AddAssign,  // a += b
  SubAssign,  // a -= b
  MulAssign,  // a *= b
  DivAssign,  // a /= b
  RemAssign,  // a %= b
  AndAssign,  // a &= b
  OrAssign,   // a |= b
  XorAssign,  // a ^= b
  ShlAssign,  // a <<= b
  ShrAssign,  // a >>= b

  PreInc,   // ++a
  PostInc,  // a++
  PreDec,   // --a
  PostDec,  // a--
};

/// Executes an overloadable operator on the given arguments.
///
/// When args has three or more elements, this will perform a fold of the form
/// (... op args), if the operation supports folding.
template <best::op op>
constexpr auto operate(auto &&...args)
    -> decltype(ops_internal::run<best::op>(ops_internal::tag<op>{},
                                            BEST_FWD(args)...)) {
  return ops_internal::run<best::op>(ops_internal::tag<op>{},
                                     BEST_FWD(args)...);
}

/// Infers the result type of `best::operate<op>(Args...)`.
template <best::op op, typename... Args>
using op_output = decltype(best::operate<op>(best::lie<Args>...));

/// Whether best::operate<op>(args...) is well-formed.
template <best::op op, typename... Args>
concept has_op = requires(Args &&...args) {
  { best::operate<op>(BEST_FWD(args)...) };
};

/// Whether best::operate<op>(args...) is well-formed and has output converting
/// to R.
template <best::op op, typename R, typename... Args>
concept has_op_r = requires(Args &&...args) {
  { best::operate<op>(BEST_FWD(args)...) } -> best::converts_to<R>;
};
}  // namespace best

#endif  // BEST_META_OPS_H_
