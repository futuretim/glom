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

#ifndef GLOM_UTILS_H
#define GLOM_UTILS_H

#include "data_structure/field.h"
#include "data_structure/numeric_format.h"

#include "data_structure/layout/layoutitem_field.h"
#include <libxml++/libxml++.h>

namespace GlomUtils
{

Glib::ustring trim_whitespace(const Glib::ustring& text);

typedef std::vector<LayoutItem_Field> type_vecLayoutFields;

  //TODO: Move this to its own file:
Glib::ustring build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause = Glib::ustring(), const Glib::ustring& sort_clause = Glib::ustring());


typedef std::list< std::pair<Gnome::Gda::Value, Gnome::Gda::Value> > type_list_values_with_second;
type_list_values_with_second get_choice_values(const LayoutItem_Field& field);


void transform_and_open(const xmlpp::Document& xml_document, const Glib::ustring& xsl_file_path);
Glib::ustring xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt);

} //namespace GlomUtils

#endif //GLOM_UTILS_H

