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

#ifndef GLOM_UTILITY_WIDGETS_DATAWIDGET_H
#define GLOM_UTILITY_WIDGETS_DATAWIDGET_H

#include "placeholder.h"
#include <gtkmm/label.h>
#include "../data_structure/field.h"


class DataWidget : public PlaceHolder
{
public:
  //explicit DataWidget(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  explicit DataWidget(Field::glom_field_type glom_type = Field::TYPE_TEXT, const Glib::ustring& title = Glib::ustring());
  virtual ~DataWidget();

  virtual Gtk::Label* get_label();
  virtual const Gtk::Label* get_label() const;
  
  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  //virtual void set_text(const Glib::ustring& text); //override

  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

  virtual void set_editable(bool editable = true);

  typedef sigc::signal<void, const Gnome::Gda::Value&> type_signal_edited;
  type_signal_edited signal_edited();

protected:

  //Overrides of default signal handlers:
  virtual void on_widget_edited(); //From Gtk::Entry, or Gtk::CheckButton.

  int get_suitable_width(Field::glom_field_type field_type);
    
  type_signal_edited m_signal_edited;

  Field::glom_field_type m_glom_type; //Store the type so we can validate the text accordingly.
  Gtk::Label m_label;
};

#endif //GLOM_UTILITY_WIDGETS_DATAWIDGET_H

