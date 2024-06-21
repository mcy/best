#ifndef BEST_META_INTERNAL_REFLECT_H_
#define BEST_META_INTERNAL_REFLECT_H_

#include "best/base/fwd.h"
#include "best/meta/taxonomy.h"
#include "best/text/str.h"

namespace best::reflect_internal {
enum kind { Field, Struct, Value, Enum, NoFields };

struct validator {
  /// This needs to be in a struct so the info classes can befriend it.
  template <typename Info, typename For>
  static constexpr bool value =
      (Info::Kind == Struct && best::same<For, typename Info::struct_>) ||
      (Info::Kind == Enum && best::same<For, typename Info::enum_>) ||
      (Info::Kind == NoFields && (best::is_struct<For> || best::is_enum<For>));
};

template <typename Info, typename For>
concept valid_reflection = validator::value<Info, For>;

template <typename S, typename T, typename... Tags>
class field_info final {
  template <auto&>
  friend class best::reflected_field;
  template <auto&>
  friend class best::reflected_type;
  friend mirror;
  friend validator;

  using struct_ = S;
  using type = T;
  inline static constexpr auto Kind = Field;

  constexpr field_info(best::str name, type struct_::*ptr,
                       best::row<Tags...> tags)
      : name_(name), ptr_(ptr), tags_(tags) {}

  best::str name_;
  type struct_::*ptr_;
  best::row<Tags...> tags_;
};

template <typename S, typename Tags, typename... Fields>
class struct_info final {
  template <auto&>
  friend class best::reflected_type;
  friend mirror;
  friend validator;

  using struct_ = S;
  using enum_ = void;
  inline static constexpr auto Kind = Struct;

  constexpr struct_info(best::str name, Tags tags, best::row<Fields...> fields)
      : name_(name), tags_(tags), items_(fields) {}

  best::str name_;
  Tags tags_;
  best::row<Fields...> items_;
};

template <typename E, typename... Tags>
class elem_info final {
  template <auto&>
  friend class best::reflected_value;
  template <auto&>
  friend class best::reflected_type;
  friend mirror;
  friend validator;

  using enum_ = E;
  inline static constexpr auto Kind = Value;

  constexpr elem_info(best::str name, enum_ elem, best::row<Tags...> tags)
      : name_(name), elem_(elem), tags_(tags) {}

  best::str name_;
  enum_ elem_;
  best::row<Tags...> tags_;
};

template <typename E, typename Tags, typename... Elems>
class enum_info final {
  template <auto&>
  friend class best::reflected_type;
  friend mirror;
  friend validator;

  using struct_ = void;
  using enum_ = E;
  inline static constexpr auto Kind = Enum;

  constexpr enum_info(best::str name, Tags tags, best::row<Elems...> values)
      : name_(name), tags_(tags), items_(values) {}

  best::str name_;
  Tags tags_;
  best::row<Elems...> items_;
};

template <typename Tags>
class no_fields final {
  template <auto&>
  friend class best::reflected_type;
  friend mirror;
  friend validator;

  using struct_ = void;
  using enum_ = void;
  inline static constexpr auto Kind = NoFields;

  constexpr no_fields(best::str name, Tags tags) : name_(name), tags_(tags) {}

  best::str name_;
  Tags tags_;
  best::row<> items_;
};

template <typename T, typename mirror = best::mirror>
inline constexpr auto info = BestReflect(mirror::BEST_MIRROR_FTADLE_, (T*){});

};  // namespace best::reflect_internal

#endif  // BEST_META_INTERNAL_REFLECT_H_