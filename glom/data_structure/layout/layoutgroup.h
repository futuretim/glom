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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUTGROUP_H
#define GLOM_DATASTRUCTURE_LAYOUTGROUP_H

#include "layoutitem_field.h"
#include <map>

class LayoutItem_Field;

class LayoutGroup : public LayoutItem
{
public:

  LayoutGroup();
  LayoutGroup(const LayoutGroup& src);
  LayoutGroup& operator=(const LayoutGroup& src);
  virtual ~LayoutGroup();

  virtual LayoutItem* clone() const;

  bool has_field(const Glib::ustring& field_name) const;

  sharedptr<LayoutItem> add_item(const sharedptr<LayoutItem>& item);
  sharedptr<LayoutItem> add_item(const sharedptr<LayoutItem>& item, guint sequence);
//  void add_item(const LayoutGroup& item);
//  void add_item(const LayoutGroup& item, guint sequence);

  virtual void change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);
  virtual void change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);

  void remove_all_items();

  //virtual void debug(guint level = 0) const;

  guint m_columns_count;

  typedef std::map<int, sharedptr<LayoutItem> > type_map_items;
  type_map_items get_items();

  typedef std::map<int, sharedptr<const LayoutItem> > type_map_const_items;
  type_map_const_items get_items() const;

  virtual Glib::ustring get_part_type_name() const;

//Allow more efficient access: protected:

  type_map_items m_map_items;

protected:

  void remove_item(guint sequence);
};

#endif //GLOM_DATASTRUCTURE_LAYOUTGROUP_H



