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

#include "best/text/format.h"

#include "best/text/utf32.h"

namespace best {
best::row<size_t, size_t> format_spec::compute_padding(
    size_t runes_to_write, align default_alignment) const {
  size_t padding = best::saturating_sub(width, runes_to_write);
  switch (alignment.value_or(default_alignment)) {
    case Left:
      return {0, padding};
    case Center:
      return {padding / 2, (padding + 1) / 2};
    case Right:
      return {padding, 0};
      break;
  }
}

void formatter::write(rune r) {
  if (r != '\n') update_indent();
  out_->push_lossy(r);
  at_new_line_ = r == '\n';
}

formatter::block formatter::list(best::str title) {
  return {{.title = title, .open = "[", .close = "]"}, this};
}
formatter::block formatter::tuple(best::str title) {
  return {{.title = title, .open = "(", .close = ")"}, this};
}
formatter::block formatter::record(best::str title) {
  return {{.title = title, .open = "{", .close = "}"}, this};
}

void formatter::update_indent() {
  if (!std::exchange(at_new_line_, false)) return;
  for (size_t i = 0; i < indent_; ++i) {
    out_->push_lossy(config_.indent);
  }
}

void formatter::format_impl(best::str templ, vptr* vtable) {
  format_internal::visit_template(
      templ.data(), templ.size(),
      [&](best::str chunk) {
        write(chunk);
        return true;
      },
      [&](size_t idx, const auto& spec) {
        vtable[idx].fn(*this, spec, vtable[idx].data);
        return true;
      });
}

void formatter::block::finish() {
  if (!fmt_) return;
  if (uses_indent_ && entries_ > 0) {
    fmt_->write(",\n");
  }
  if (uses_indent_) --fmt_->indent_;
  fmt_->write(config_.close);
}

void formatter::block::advise_size(size_t n) {
  if (!fmt_) return;
  if (n == 1 && uses_indent_) {
    uses_indent_ = false;
    --fmt_->indent_;
  }
}

formatter::block::block(const config& config, formatter* fmt)
    : config_(config), fmt_(fmt) {
  if (!config_.title.is_empty()) {
    fmt_->write(config_.title);
    fmt_->write(' ');
  }
  fmt_->write(config.open);

  uses_indent_ = fmt_->current_spec().alt;
  if (uses_indent_) ++fmt_->indent_;
}

void formatter::block::separator() {
  if (entries_ > 0) fmt_->write(",");
  if (uses_indent_) fmt_->write("\n");
  if (!uses_indent_ && entries_ > 0) fmt_->write(" ");
  ++entries_;
}

template void BestFmt(best::formatter&, const char*);
template void BestFmt(best::formatter&, const char16_t*);
template void BestFmt(best::formatter&, const char32_t*);
template void BestFmt(best::formatter&, const best::pretext<utf8>&);
template void BestFmt(best::formatter&, const best::text<utf8>&);
template void BestFmt(best::formatter&, const best::pretext<utf16>&);
template void BestFmt(best::formatter&, const best::text<utf16>&);
template void BestFmt(best::formatter&, const best::pretext<wtf8>&);
template void BestFmt(best::formatter&, const best::text<wtf8>&);
template void BestFmt(best::formatter&, char);
template void BestFmt(best::formatter&, signed char);
template void BestFmt(best::formatter&, signed short);
template void BestFmt(best::formatter&, signed int);
template void BestFmt(best::formatter&, signed long);
template void BestFmt(best::formatter&, signed long long);
template void BestFmt(best::formatter&, unsigned char);
template void BestFmt(best::formatter&, unsigned short);
template void BestFmt(best::formatter&, unsigned int);
template void BestFmt(best::formatter&, unsigned long);
template void BestFmt(best::formatter&, unsigned long long);

template void formatter::write(const best::pretext<utf8>&);
template void formatter::write(const best::text<utf8>&);
template void formatter::write(const best::pretext<wtf8>&);
template void formatter::write(const best::text<wtf8>&);
template void formatter::write(const best::pretext<utf16>&);
template void formatter::write(const best::text<utf16>&);
}  // namespace best
