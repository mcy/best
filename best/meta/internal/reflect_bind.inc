// GENERATED CODE! DO NOT EDIT! See reflect_bind.sh.
#define BEST_REFLECT_MAX_FIELDS_ 64
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<0>)
requires requires { best::as_auto<decltype(val)>{}; } {
  return best::call(BEST_FWD(cb));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<1>)
requires requires { best::as_auto<decltype(val)>{any}; } {
  auto&& [_0] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<2>)
requires requires { best::as_auto<decltype(val)>{any, any }; } {
  auto&& [_0, _1] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<3>)
requires requires { best::as_auto<decltype(val)>{any, any , any }; } {
  auto&& [_0, _1, _2] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<4>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any }; } {
  auto&& [_0, _1, _2, _3] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<5>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<6>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<7>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<8>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<9>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<10>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<11>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<12>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<13>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<14>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<15>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<16>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<17>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<18>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<19>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<20>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<21>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<22>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<23>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<24>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<25>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<26>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<27>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<28>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<29>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<30>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<31>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<32>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<33>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<34>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<35>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<36>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<37>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<38>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<39>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<40>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<41>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<42>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<43>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<44>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<45>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<46>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<47>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<48>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<49>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<50>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<51>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<52>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<53>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<54>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<55>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<56>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<57>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<58>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56), BEST_FWD(_57));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<59>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56), BEST_FWD(_57), BEST_FWD(_58));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<60>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56), BEST_FWD(_57), BEST_FWD(_58), BEST_FWD(_59));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<61>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56), BEST_FWD(_57), BEST_FWD(_58), BEST_FWD(_59), BEST_FWD(_60));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<62>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56), BEST_FWD(_57), BEST_FWD(_58), BEST_FWD(_59), BEST_FWD(_60), BEST_FWD(_61));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<63>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56), BEST_FWD(_57), BEST_FWD(_58), BEST_FWD(_59), BEST_FWD(_60), BEST_FWD(_61), BEST_FWD(_62));
}
constexpr decltype(auto) bind(auto&& val, auto&& cb, rank<64>)
requires requires { best::as_auto<decltype(val)>{any, any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any , any }; } {
  auto&& [_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63] = BEST_FWD(val);
  return best::call(BEST_FWD(cb), BEST_FWD(_0), BEST_FWD(_1), BEST_FWD(_2), BEST_FWD(_3), BEST_FWD(_4), BEST_FWD(_5), BEST_FWD(_6), BEST_FWD(_7), BEST_FWD(_8), BEST_FWD(_9), BEST_FWD(_10), BEST_FWD(_11), BEST_FWD(_12), BEST_FWD(_13), BEST_FWD(_14), BEST_FWD(_15), BEST_FWD(_16), BEST_FWD(_17), BEST_FWD(_18), BEST_FWD(_19), BEST_FWD(_20), BEST_FWD(_21), BEST_FWD(_22), BEST_FWD(_23), BEST_FWD(_24), BEST_FWD(_25), BEST_FWD(_26), BEST_FWD(_27), BEST_FWD(_28), BEST_FWD(_29), BEST_FWD(_30), BEST_FWD(_31), BEST_FWD(_32), BEST_FWD(_33), BEST_FWD(_34), BEST_FWD(_35), BEST_FWD(_36), BEST_FWD(_37), BEST_FWD(_38), BEST_FWD(_39), BEST_FWD(_40), BEST_FWD(_41), BEST_FWD(_42), BEST_FWD(_43), BEST_FWD(_44), BEST_FWD(_45), BEST_FWD(_46), BEST_FWD(_47), BEST_FWD(_48), BEST_FWD(_49), BEST_FWD(_50), BEST_FWD(_51), BEST_FWD(_52), BEST_FWD(_53), BEST_FWD(_54), BEST_FWD(_55), BEST_FWD(_56), BEST_FWD(_57), BEST_FWD(_58), BEST_FWD(_59), BEST_FWD(_60), BEST_FWD(_61), BEST_FWD(_62), BEST_FWD(_63));
}
