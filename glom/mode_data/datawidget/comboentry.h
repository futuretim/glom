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

#ifndef GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H
#define GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/mode_data/datawidget/combochoiceswithtreemodel.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/picker-button.h>
#include <hildonmm/touch-selector-entry.h>
#endif //GLOM_ENABLE_MAEMO

namespace Glom
{

class Application;

namespace DataWidgetChildren
{

/** A Gtk::ComboBox that can show choices of field values.
 * Use this when the user should be allowed to enter values directly too,
 * including values that are not in the choices.
 */
class ComboEntry
:
#ifndef GLOM_ENABLE_MAEMO
  public Gtk::ComboBox,
#else
  public Hildon::PickerButton,
#endif
  public ComboChoicesWithTreeModel
{
public:
  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  ComboEntry();

  virtual ~ComboEntry();

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  void set_text(const Glib::ustring& text); //override

  /** Set the text from a Gnome::Gda::Value.
   */
  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

  virtual void set_layout_item(const sharedptr<LayoutItem>& layout_item, const Glib::ustring& table_name);

  virtual void set_read_only(bool read_only = true);

  virtual void set_choices_related(const Document* document, const sharedptr<const Relationship>& relationship, const Glib::ustring& field, const Glib::ustring& field_second, bool show_all);

private:
  void init();


  //Overrides of default signal handlers:
  //TODO: Are these really default signal handlers?
  virtual void on_entry_changed(); //From Gtk::Entry.
  virtual void on_entry_activate(); //From Gtk::Entry.
  virtual bool on_entry_focus_out_event(GdkEventFocus* event); //From Gtk::Widget


  #ifndef GLOM_ENABLE_MAEMO
  // Note that this is a normal signal handler when glibmm was complied
  // without default signal handlers
  virtual void on_changed(); //From Gtk::ComboBox
  #else
  void on_changed(int column);
  #endif //GLOM_ENABLE_MAEMO


  virtual void check_for_change();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool on_entry_button_press_event(GdkEventButton *event);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual Application* get_application();

  Gtk::Entry* get_entry();
  const Gtk::Entry* get_entry() const;

  Glib::ustring m_old_text;
  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.

  #ifdef GLOM_ENABLE_MAEMO
  Hildon::TouchSelectorEntry m_maemo_selector;
  #endif

  Gtk::CellRenderer* m_cell_second;
};

} //namespace DataWidetChildren
} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H