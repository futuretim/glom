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

#include "textviewglom.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <gtkmm/messagedialog.h>
#include "../dialog_invalid_data.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include "../application.h"
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl

namespace Glom
{

TextViewGlom::TextViewGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::ScrolledWindow(cobject),
  m_glom_type(Field::TYPE_TEXT)
{
  init();
}

TextViewGlom::TextViewGlom(Field::glom_field_type glom_type)
: m_glom_type(glom_type)
{
  init();
}

void TextViewGlom::init()
{
  setup_menu();

  set_shadow_type(Gtk::SHADOW_IN);

  m_TextView.show();
  add(m_TextView);

  //Wrap text, and allow vertical scrolling:
  set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC); 
  set_shadow_type(Gtk::SHADOW_IN);
  m_TextView.set_wrap_mode(Gtk::WRAP_WORD);

  //We use connect(slot, false) to connect before the default signal handler, because the default signal handler prevents _further_ handling.
  m_TextView.signal_focus_out_event().connect(sigc::mem_fun(*this, &TextViewGlom::on_textview_focus_out_event), false);
  // m_TextView.get_buffer()->signal_end_user_action().connect(sigc::mem_fun(*this, &TextViewGlom::on_buffer_changed));
}

TextViewGlom::~TextViewGlom()
{
}

void TextViewGlom::set_glom_type(Field::glom_field_type glom_type)
{
  m_glom_type = glom_type;
}

void TextViewGlom::check_for_change()
{
  const Glib::ustring new_text = m_TextView.get_buffer()->get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    sharedptr<const LayoutItem_Field>layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
    Gnome::Gda::Value value = GlomConversions::parse_value(m_glom_type, new_text, layout_item->get_formatting_used().m_numeric_format, success);

    if(success)
    {
      //Actually show the canonical text:
      set_value(value);
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //Tell the user and offer to revert or try again:
      bool revert = glom_show_dialog_invalid_data(m_glom_type);
      if(revert)
      {
        set_text(m_old_text);
      }
      else
        grab_focus(); //Force the user back into the same field, so that the field can be checked again and eventually corrected or reverted.
    }
  }
}


bool TextViewGlom::on_textview_focus_out_event(GdkEventFocus* event)
{
  //Call base class:
  bool result = Gtk::ScrolledWindow::on_focus_out_event(event);

  //The user has finished editing.
  check_for_change();

  return result;
}


/*

void TextViewGlom::on_activate()
{ 
  //Call base class:
  Gtk::TextView::on_activate();

  //The user has finished editing.
  check_for_change();
}
*/

void TextViewGlom::on_buffer_changed()
{
  check_for_change();
}

/*
void TextViewGlom::on_insert_text(const Glib::ustring& text, int* position)
{
  Gtk::TextView::on_insert_text(text, position);
}

*/

void TextViewGlom::set_value(const Gnome::Gda::Value& value)
{
  sharedptr<const LayoutItem_Field>layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(layout_item)
    set_text(GlomConversions::get_text_for_gda_value(m_glom_type, value, layout_item->get_formatting_used().m_numeric_format));
}

void TextViewGlom::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  //Call base class:
  m_TextView.get_buffer()->set_text(text);
}

Gnome::Gda::Value TextViewGlom::get_value() const
{
  bool success = false;

  sharedptr<const LayoutItem_Field>layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());

  TextViewGlom* pNonConstThis = const_cast<TextViewGlom*>(this); //Gtk::TextBuffer::get_text() is non-const in gtkmm <=2.6.
  return GlomConversions::parse_value(m_glom_type, pNonConstThis->m_TextView.get_buffer()->get_text(true), layout_item->get_formatting_used().m_numeric_format, success);
}

bool TextViewGlom::on_button_press_event(GdkEventButton *event)
{
  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      GdkModifierType mods;
      gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }

  }

  return Gtk::ScrolledWindow::on_button_press_event(event);
}

App_Glom* TextViewGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

} //namespace Glom
