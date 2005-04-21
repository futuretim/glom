/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H
#define GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H

#include "../data_structure/field.h"
#include "../data_structure/numeric_format.h"

#include "../data_structure/layout/layoutitem_field.h"

namespace GlomConversions
{
  ///Get text for display to the user.
  Glib::ustring get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const NumericFormat& numeric_format = NumericFormat());
  Glib::ustring get_text_for_gda_value(Field::glom_field_type glom_type, const Gnome::Gda::Value& value, const std::locale& locale, const NumericFormat& numeric_format = NumericFormat(), bool iso_format = false);

  Glib::ustring format_time(const tm& tm_data);
  Glib::ustring format_time(const tm& tm_data, const std::locale& locale, bool iso_format = false);
  Glib::ustring format_date(const tm& tm_data);
  Glib::ustring format_date(const tm& tm_data, const std::locale& locale, bool iso_format = false);

  Gnome::Gda::Value parse_value(double number);
  Gnome::Gda::Value parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, bool& success, bool iso_format = false);
  Gnome::Gda::Value parse_value(Field::glom_field_type glom_type, const Glib::ustring& text, const NumericFormat& numeric_format, bool& success, bool iso_format = false);

  tm parse_date(const Glib::ustring& text, bool& success);
  tm parse_date(const Glib::ustring& text, const std::locale& locale, bool& success);
  tm parse_time(const Glib::ustring& text, bool& success);
  tm parse_time(const Glib::ustring& text, const std::locale& locale, bool& success);

  Glib::ustring format_tm(const tm& tm_data, const std::locale& locale, char format);
  //static tm parse_tm(const Glib::ustring& text, const std::locale& locale, char format);

  bool value_is_empty(const Gnome::Gda::Value& value);
  Gnome::Gda::Value get_empty_value(Field::glom_field_type field_type);

  Gnome::Gda::Value get_example_value(Field::glom_field_type field_type);

  Glib::ustring util_trim_whitespace(const Glib::ustring& text);
}

typedef std::vector<LayoutItem_Field> type_vecLayoutFields;

//TODO: Move this to its own file:
Glib::ustring util_build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause = Glib::ustring());


typedef std::list< std::pair<Gnome::Gda::Value, Gnome::Gda::Value> > type_list_values_with_second;
type_list_values_with_second get_choice_values(const LayoutItem_Field& field);


#endif //GLOM_DATASTRUCTURE_GLOMCONVERSIONS_H

