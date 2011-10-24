/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#ifndef GLOM_MODE_DATA_LAYOUT_WIDGET_MENU_H
#define GLOM_MODE_DATA_LAYOUT_WIDGET_MENU_H

#include "layoutwidgetbase.h"
#include <gtkmm/uimanager.h>

namespace Glom
{

class LayoutWidgetMenu : public LayoutWidgetBase
{
public: 
  LayoutWidgetMenu();
  virtual ~LayoutWidgetMenu();
  
  //Popup-menu:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void setup_menu();
  virtual void on_menupopup_activate_layout();
  virtual void on_menupopup_activate_layout_properties();
  virtual void on_menupopup_add_item(enumType item);
  virtual void on_menupopup_activate_delete();
#endif // !GLOM_ENABLE_CLIENT_ONLY  
    
protected:
#ifndef GLOM_ENABLE_CLIENT_ONLY    
  Gtk::Menu* m_pMenuPopup;

  //TODO_Performance: //Presumably we waste lots of memory by having this in each layout widget. Maybe we can use one shared menu.
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;

  Glib::RefPtr<Gtk::Action> m_refContextLayout, m_refContextLayoutProperties;
  Glib::RefPtr<Gtk::Action> m_refContextAddField, m_refContextAddRelatedRecords, 
    m_refContextAddGroup, m_refContextAddNotebook, m_refContextAddButton, m_refContextAddText;
  Glib::RefPtr<Gtk::Action> m_refContextDelete;
#endif // GLOM_ENABLE_CLIENT_ONLY
};

} //namespace Glom

#endif //GLOM_MODE_DATA_LAYOUT_WIDGET_MENU_H
