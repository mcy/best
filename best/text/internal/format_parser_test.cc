#include "best/text/internal/format_parser.h"

#include "best/container/vec.h"
#include "best/test/test.h"
#include "best/text/format.h"

namespace best::format_internal::parser_test {
using Node = best::choice<best::str, best::row<size_t, best::format_spec>>;
using Ast = best::vec<Node>;

best::option<Ast> parse(best::str templ) {
  Ast out;
  bool ok = visit_template(
      templ,
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
              Ast{{best::row{0, format_spec{}}.forward()}});

  t.expect_eq(parse("{} "),  //
              Ast{{best::row{0, format_spec{}}.forward(), best::str(" ")}});

  t.expect_eq(parse(" {}"),  //
              Ast{{best::str(" "), best::row{0, format_spec{}}.forward()}});

  t.expect_eq(parse("hello, {}!"),  //
              Ast{{
                  best::str{"hello, "},
                  best::row{0, format_spec{}}.forward(),
                  best::str("!"),
              }});

  t.expect_eq(parse("hello, {}, {}, {}!"),
              Ast{{
                  best::str{"hello, "},
                  best::row{0, format_spec{}}.forward(),
                  best::str(", "),
                  best::row{1, format_spec{}}.forward(),
                  best::str(", "),
                  best::row{2, format_spec{}}.forward(),
                  best::str("!"),
              }});

  t.expect_eq(parse("hello, {1}, {}, {}!"),
              Ast{{
                  best::str{"hello, "},
                  best::row{1, format_spec{}}.forward(),
                  best::str(", "),
                  best::row{0, format_spec{}}.forward(),
                  best::str(", "),
                  best::row{1, format_spec{}}.forward(),
                  best::str("!"),
              }});

  t.expect_eq(parse("align: {:x<1} {5:0<1} {:<1} {:<<1} {:x^1} {5:0^1} {:^1} "
                    "{:^^1} {:x>1} "
                    "{5:0>1} {:>1} {:>>1}"),  //
              Ast{{
                  best::str{"align: "},
                  best::row{0, format_spec{.alignment = format_spec::Left,
                                           .fill = 'x',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{5, format_spec{.alignment = format_spec::Left,
                                           .fill = '0',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{1, format_spec{.alignment = format_spec::Left,
                                           .fill = ' ',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{2, format_spec{.alignment = format_spec::Left,
                                           .fill = '<',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{3, format_spec{.alignment = format_spec::Center,
                                           .fill = 'x',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{5, format_spec{.alignment = format_spec::Center,
                                           .fill = '0',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{4, format_spec{.alignment = format_spec::Center,
                                           .fill = ' ',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{5, format_spec{.alignment = format_spec::Center,
                                           .fill = '^',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{6, format_spec{.alignment = format_spec::Right,
                                           .fill = 'x',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{5, format_spec{.alignment = format_spec::Right,
                                           .fill = '0',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{7, format_spec{.alignment = format_spec::Right,
                                           .fill = ' ',
                                           .width = 1}}
                      .forward(),
                  best::str(" "),
                  best::row{8, format_spec{.alignment = format_spec::Right,
                                           .fill = '>',
                                           .width = 1}}
                      .forward(),
              }});

  t.expect_eq(
      parse("flags: {:#} {:?} {:#?}"),
      Ast{{
          best::str{"flags: "},
          best::row{0, format_spec{.alt = true}}.forward(),
          best::str(" "),
          best::row{1, format_spec{.debug = true}}.forward(),
          best::str(" "),
          best::row{2, format_spec{.alt = true, .debug = true}}.forward(),
      }});

  t.expect_eq(
      parse("widths: {:5} {:05} {:<5} {:<05}"),
      Ast{{
          best::str{"widths: "},
          best::row{0, format_spec{.alignment = format_spec::Right, .width = 5}}
              .forward(),
          best::str(" "),
          best::row{1, format_spec{.sign_aware_padding = true,
                                   .alignment = format_spec::Right,
                                   .width = 5}}
              .forward(),
          best::str(" "),
          best::row{2, format_spec{.alignment = format_spec::Left, .width = 5}}
              .forward(),
          best::str(" "),
          best::row{3, format_spec{.sign_aware_padding = true,
                                   .alignment = format_spec::Left,
                                   .width = 5}}
              .forward(),
      }});

  t.expect_eq(
      parse("precs: {:.2} {:5.2} {:05.2}"),
      Ast{{
          best::str{"precs: "},
          best::row{0, format_spec{.prec = 2}}.forward(),
          best::str(" "),
          best::row{1, format_spec{.width = 5, .prec = 2}}.forward(),
          best::str(" "),
          best::row{
              2, format_spec{.sign_aware_padding = true, .width = 5, .prec = 2}}
              .forward(),
      }});

  t.expect_eq(parse("methods: {:x} {:o} {:A} {:x?}"),
              Ast{{
                  best::str{"methods: "},
                  best::row{0, format_spec{.method = rune('x')}}.forward(),
                  best::str(" "),
                  best::row{1, format_spec{.method = rune('o')}}.forward(),
                  best::str(" "),
                  best::row{2, format_spec{.method = rune('A')}}.forward(),
                  best::str(" "),
                  best::row{3, format_spec{.debug = true, .method = rune('x')}}
                      .forward(),
              }});
};
}  // namespace best::format_internal::parser_test