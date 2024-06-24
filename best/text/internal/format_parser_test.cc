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

#include "best/text/internal/format_parser.h"

#include "best/container/vec.h"
#include "best/test/test.h"
#include "best/text/format.h"

namespace best {
void BestFmt(auto& fmt, const format_spec& spec) {
  auto rec = fmt.record();
  rec.field("alt", spec.alt);
  rec.field("debug", spec.debug);
  rec.field("sign_aware_padding", spec.sign_aware_padding);
  rec.field("alignment", best::option<int>(spec.alignment));
  rec.field("fill", spec.fill);
  rec.field("width", spec.width);
  rec.field("prec", spec.prec);
  rec.field("method", spec.method);
}
}  // namespace best

namespace best::format_internal::parser_test {
using Node = best::choice<best::str, best::row<size_t, best::format_spec>>;
using Ast = best::vec<Node>;

best::option<Ast> parse(best::str templ) {
  Ast out;
  bool ok = visit_template(
      templ.data(), templ.size(),
      [&out](best::str s) {
        out.push(s);
        return true;
      },
      [&out](size_t n, const best::format_spec& spec) {
        out.push(best::row{n, spec});
        return true;
      });

  if (!ok) return best::none;
  return out;
}

best::test ParseOk = [](auto& t) {
  t.expect_eq(parse("hello, world!"),  //
              Ast{best::str{"hello, world!"}});

  t.expect_eq(parse("hello, {{braces}}!"),  //
              Ast{best::str{"hello, "}, best::str("{"), best::str("braces"),
                  best::str("}"), best::str("!")});

  t.expect_eq(parse("{}"),  //
              Ast{{best::args(0, format_spec{})}});

  t.expect_eq(parse("{} "),  //
              Ast{{best::args(0, format_spec{}), best::str(" ")}});

  t.expect_eq(parse(" {}"),  //
              Ast{{best::str(" "), best::args(0, format_spec{})}});

  t.expect_eq(parse("hello, {}!"),  //
              Ast{{
                  best::str{"hello, "},
                  best::args(0, format_spec{}),
                  best::str("!"),
              }});

  t.expect_eq(parse("hello, {}, {}, {}!"), Ast{{
                                               best::str{"hello, "},
                                               best::args(0, format_spec{}),
                                               best::str(", "),
                                               best::args(1, format_spec{}),
                                               best::str(", "),
                                               best::args(2, format_spec{}),
                                               best::str("!"),
                                           }});

  t.expect_eq(parse("hello, {1}, {}, {}!"), Ast{{
                                                best::str{"hello, "},
                                                best::args(1, format_spec{}),
                                                best::str(", "),
                                                best::args(0, format_spec{}),
                                                best::str(", "),
                                                best::args(1, format_spec{}),
                                                best::str("!"),
                                            }});

  t.expect_eq(parse("align: {:x<1} {5:0<1} {:<1} {:<<1} {:x^1} {5:0^1} {:^1} "
                    "{:^^1} {:x>1} "
                    "{5:0>1} {:>1} {:>>1}"),  //
              Ast{{
                  best::str{"align: "},
                  best::args(0, format_spec{.alignment = format_spec::Left,
                                            .fill = 'x',
                                            .width = 1}),
                  best::str(" "),
                  best::args(5, format_spec{.alignment = format_spec::Left,
                                            .fill = '0',
                                            .width = 1}),
                  best::str(" "),
                  best::args(1, format_spec{.alignment = format_spec::Left,
                                            .fill = ' ',
                                            .width = 1}),
                  best::str(" "),
                  best::args(2, format_spec{.alignment = format_spec::Left,
                                            .fill = '<',
                                            .width = 1}),
                  best::str(" "),
                  best::args(3, format_spec{.alignment = format_spec::Center,
                                            .fill = 'x',
                                            .width = 1}),
                  best::str(" "),
                  best::args(5, format_spec{.alignment = format_spec::Center,
                                            .fill = '0',
                                            .width = 1}),
                  best::str(" "),
                  best::args(4, format_spec{.alignment = format_spec::Center,
                                            .fill = ' ',
                                            .width = 1}),
                  best::str(" "),
                  best::args(5, format_spec{.alignment = format_spec::Center,
                                            .fill = '^',
                                            .width = 1}),
                  best::str(" "),
                  best::args(6, format_spec{.alignment = format_spec::Right,
                                            .fill = 'x',
                                            .width = 1}),
                  best::str(" "),
                  best::args(5, format_spec{.alignment = format_spec::Right,
                                            .fill = '0',
                                            .width = 1}),
                  best::str(" "),
                  best::args(7, format_spec{.alignment = format_spec::Right,
                                            .fill = ' ',
                                            .width = 1}),
                  best::str(" "),
                  best::args(8, format_spec{.alignment = format_spec::Right,
                                            .fill = '>',
                                            .width = 1}),
              }});

  t.expect_eq(parse("flags: {:#} {:?} {:#?}"),
              Ast{{
                  best::str{"flags: "},
                  best::args(0, format_spec{.alt = true}),
                  best::str(" "),
                  best::args(1, format_spec{.debug = true}),
                  best::str(" "),
                  best::args(2, format_spec{.alt = true, .debug = true}),
              }});

  t.expect_eq(
      parse("widths: {:5} {:05} {:<5}"),
      Ast{{
          best::str{"widths: "},
          best::args(0, format_spec{.width = 5}),
          best::str(" "),
          best::args(1, format_spec{.sign_aware_padding = true, .width = 5}),
          best::str(" "),
          best::args(2,
                     format_spec{.alignment = format_spec::Left, .width = 5}),
      }});

  t.expect_eq(parse("precs: {:.2} {:5.2} {:05.2}"),
              Ast{{
                  best::str{"precs: "},
                  best::args(0, format_spec{.prec = 2}),
                  best::str(" "),
                  best::args(1, format_spec{.width = 5, .prec = 2}),
                  best::str(" "),
                  best::args(2, format_spec{.sign_aware_padding = true,
                                            .width = 5,
                                            .prec = 2}),
              }});

  t.expect_eq(
      parse("methods: {:x} {:o} {:A} {:x?}"),
      Ast{{
          best::str{"methods: "},
          best::args(0, format_spec{.method = rune('x')}),
          best::str(" "),
          best::args(1, format_spec{.method = rune('o')}),
          best::str(" "),
          best::args(2, format_spec{.method = rune('A')}),
          best::str(" "),
          best::args(3, format_spec{.debug = true, .method = rune('x')}),
      }});
};
}  // namespace best::format_internal::parser_test
