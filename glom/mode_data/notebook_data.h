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

#ifndef NOTEBOOK_DATA_H
#define NOTEBOOK_DATA_H

#include "../notebook_glom.h"
#include "box_data_list.h"
#include "box_data_details.h"

namespace Glom
{

class Notebook_Data : public Notebook_Glom
{
public: 
  Notebook_Data();
  virtual ~Notebook_Data();

  /** Create the layout for the database structure, and fill it with data.
   * @param found_set Specifies a found sub-set of the table's records, or all records.
   * @param primary_key_value_for_details Specifies a single record to show in the details tab, if specified.
   * @result true if the operation was successful.
   */
  bool init_db_details(const FoundSet& found_set, const Gnome::Gda::Value& primary_key_value_for_details = Gnome::Gda::Value());

  ///Get the existing where clause, previously supplied to init_db_details().
  FoundSet get_found_set() const;
  
  ///Get the found set for the currently-visible record in the details tab:
  FoundSet get_found_set_details() const;

  ///Show the details for a particular record, without affecting the list view:
  void show_details(const Gnome::Gda::Value& primary_key_value);

  void select_page_for_find_results(); //Details for 1, List for > 1.

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void do_menu_developer_layout(); //override
  void show_layout_toolbar(bool show = true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual void do_menu_file_print(); //override

  void get_record_counts(gulong& total, gulong& found);

  enum dataview
  {
    DATA_VIEW_Details,
    DATA_VIEW_List
  };

  dataview get_current_view() const;
  void set_current_view(dataview view);

  typedef sigc::signal<void, const Glib::ustring&, Gnome::Gda::Value> type_signal_record_details_requested;
  type_signal_record_details_requested signal_record_details_requested();

protected:

  ///Show the counts of all records and found records.
  void update_records_count();

  //Signal handlers:
  virtual void on_list_user_requested_details(const Gnome::Gda::Value& primary_key_value);
  void on_details_user_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);

  virtual void on_switch_page_handler(GtkNotebookPage* pPage, guint uiPageNumber);

  //Member widgets:
  Box_Data_List m_Box_List;
  Box_Data_Details m_Box_Details;

  guint m_iPage_Details, m_iPage_List;
  Glib::ustring m_table_name;
  type_signal_record_details_requested m_signal_record_details_requested;
};

} //namespace Glom

#endif
