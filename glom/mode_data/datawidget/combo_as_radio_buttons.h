/* Glom
 *
 * Copyright (C) 2010 Murray Cumming
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_UTILITY_WIDGETS_COMBO_AS_RADIO_BUTTONS_H
#define GLOM_UTILITY_WIDGETS_COMBO_AS_RADIO_BUTTONS_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/mode_data/datawidget/combochoices.h>
#include <gtkmm/box.h>
#include <gtkmm/radiobutton.h>

namespace Glom
{

class AppWindow;

namespace DataWidgetChildren
{

/** A set of radio buttons, with an API similar to a ComboBox with restricted values.
 * Use this only when the user should only be allowed to enter values that are in the choices.
 */
class ComboAsRadioButtons
:
  public Gtk::Box,
  public ComboChoices
{
public:

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  ComboAsRadioButtons();

  virtual ~ComboAsRadioButtons();

  void set_choices_fixed(const Formatting::type_list_values& list_values, bool restricted = false) override;

  void set_choices_related(const std::shared_ptr<const Document>& document, const LayoutItem_Field& layout_field, const Gnome::Gda::Value& foreign_key_value) override;

  void set_read_only(bool read_only = true) override;

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  void set_text(const Glib::ustring& text);

  Glib::ustring get_text() const;

  /** Set the text from a Gnome::Gda::Value.
   */
  void set_value(const Gnome::Gda::Value& value) override;

  Gnome::Gda::Value get_value() const override;

private:

  typedef std::vector<Gnome::Gda::Value> type_list_values;
  typedef std::vector< std::pair<Gnome::Gda::Value, type_list_values> > type_list_values_with_second;

  //A utility function that's needed because we don't use a real db model:
  void set_choices_with_second(const type_list_values_with_second& list_values);

  void on_radiobutton_toggled();

  void check_for_change();


#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool on_button_press_event(GdkEventButton *event) override;
  bool on_radiobutton_button_press_event(GdkEventButton *event);
  void show_context_menu(GdkEventButton *event);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  AppWindow* get_appwindow() const override;


  Glib::ustring m_old_text;

  std::unordered_map<Glib::ustring, Gtk::RadioButton*, std::hash<std::string>> m_map_buttons;
};

} //namespace DataWidetChildren
} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H
