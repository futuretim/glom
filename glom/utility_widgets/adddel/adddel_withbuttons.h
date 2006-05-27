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

#ifndef ADDDEL_WITHBUTTONS_H
#define ADDDEL_WITHBUTTONS_H

#include "adddel.h"

namespace Glom
{

class AddDel_WithButtons : public AddDel
{
public: 
  AddDel_WithButtons();
  AddDel_WithButtons(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~AddDel_WithButtons();

  virtual void set_allow_add(bool val = true); //override
  virtual void set_allow_delete(bool val = true); //override
  virtual void set_allow_user_actions(bool bVal = true); //override

protected:
  void init();
  virtual void setup_buttons();

  virtual void on_button_add();
  virtual void on_button_del();
  virtual void on_button_edit();

  //member widgets:
  Gtk::HBox m_HBox;
  Gtk::Button m_Button_Add;
  Gtk::Button m_Button_Del;
  Gtk::Button m_Button_Edit;
};

} //namespace Glom

#endif //ADDDEL_WITHBUTTONS_H
