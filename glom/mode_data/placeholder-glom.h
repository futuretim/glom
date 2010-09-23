/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * glom
 * Copyright (C) Johannes Schmid 2007 <jhs@gnome.org>
 * 
 * glom is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * glom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with glom.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#ifndef GLOM_MODE_DATA_PLACEHOLDER_GLOM_H
#define GLOM_MODE_DATA_PLACEHOLDER_GLOM_H

#include <gtkmm.h>
#include <glom/utility_widgets/layoutwidgetbase.h>
//#include <libglom/data_structure/layout/layoutitem_button.h>
//#include <gtkmm/builder.h>

namespace Glom
{

class PlaceholderGlom: 
  public Gtk::Widget,
  public LayoutWidgetBase
{
public:
  //explicit PlaceholderGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  explicit PlaceholderGlom();
  virtual ~PlaceholderGlom();

private:
  virtual Application* get_application();
  
  virtual void on_size_request(Gtk::Requisition* requisition);
  virtual void on_size_allocate(Gtk::Allocation& allocation);
  virtual void on_realize();
  virtual void on_unrealize();
  virtual bool on_expose_event(GdkEventExpose* event);

  Glib::RefPtr<Gdk::Window> m_refGdkWindow;
};

} // namespace Glom

#endif // GLOM_MODE_DATA_PLACEHOLDER_GLOM_H
