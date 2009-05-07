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


#ifndef BASE_DB_TABLE_DATA_H
#define BASE_DB_TABLE_DATA_H

#include "base_db_table.h"
#include <libglom/data_structure/field.h>
#include <algorithm> //find_if used in various places.

namespace Glom
{

/** A base class some database functionality 
 * for use with a specific database table, showing data from the table.
 */
class Base_DB_Table_Data : public Base_DB_Table
{
public: 
  Base_DB_Table_Data();
  virtual ~Base_DB_Table_Data();

  virtual bool refresh_data_from_database();
    
  /** Tell the parent widget that something has changed in one of the shown records,
   * or a record was added or deleted.
   * This is only emitted for widgets for which it would be useful.
   *
   * @param relationship_name, if any.
   */
  typedef sigc::signal<void> type_signal_record_changed;
  type_signal_record_changed signal_record_changed();

protected:

  typedef std::pair< sharedptr<Field>, Gnome::Gda::Value> type_field_and_value;
  typedef std::list<type_field_and_value> type_field_values; 

  /** Create a new record with all the entered field values from the currently-active details/row.
   * @param The table to which to add a new record.
   * @param use_entered_data Whether the record should contain data already entered in the UI by the user, if table_name is m_table_name.
   * @param primary_key_value The new primary key value for the new record. Otherwise the primary key value must be in the entered data or in the @a field_values parameter.
   * @param field_values Values to use for fields, instead of entered data.
   * @result true if the record was added to the database.
   */
  bool record_new(const Glib::ustring& table_name, bool use_entered_data = true, const Gnome::Gda::Value& primary_key_value = Gnome::Gda::Value(), const type_field_values& field_values = type_field_values()); 

  Gnome::Gda::Value get_entered_field_data_field_only(const sharedptr<const Field>& field) const;
  virtual Gnome::Gda::Value get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const;
    
  virtual sharedptr<Field> get_field_primary_key() const = 0;
  virtual Gnome::Gda::Value get_primary_key_value_selected() const = 0;
  virtual void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value) = 0;
  virtual Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const = 0;

  virtual void refresh_related_fields(const LayoutFieldInRecord& field_in_record_changed, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value);

  /** Get the fields that are in related tables, via a relationship using @a field_name changes.
   */
  type_vecLayoutFields get_related_fields(const sharedptr<const LayoutItem_Field>& field) const;
      
  /** Ask the user if he really wants to delete the record.
   */  
  bool confirm_delete_record();
    
  /** Delete a record from the database table.
   * @param primary_key_value A primary key to indentify the record to delete.
   */
  bool record_delete(const Gnome::Gda::Value& primary_key_value);
    
  bool add_related_record_for_field(const sharedptr<const LayoutItem_Field>& layout_item_parent, const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& primary_key_field, const Gnome::Gda::Value& primary_key_value_provided, Gnome::Gda::Value& primary_key_value_used);

  virtual void on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row); //Overridden by derived classes.
  virtual void on_record_deleted(const Gnome::Gda::Value& primary_key_value); //Overridden by derived classes.

  //Gets the row being edited, for derived classes that have rows.
  virtual Gtk::TreeModel::iterator get_row_selected();
      
  FoundSet m_found_set;

  type_vec_fields m_TableFields; //A cache, so we don't have to repeatedly get them from the Document.
  type_vecLayoutFields m_FieldsShown; //And any extra keys needed by shown fields.

  type_signal_record_changed m_signal_record_changed;
    
private:
  bool get_related_record_exists(const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value);
};

} //namespace Glom

#endif //BASE_DB_TABLE__DATAH
