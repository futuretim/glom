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

#ifndef GLOM_UTILITYWIDGETS_FLOWTABLE_H
#define GLOM_UTILITYWIDGETS_FLOWTABLE_H

#include <gtkmm.h>
#include "layoutwidgetbase.h"

namespace Glom
{

class FlowTable : public Gtk::Container
{
public: 
  FlowTable();
  virtual ~FlowTable();

  typedef Gtk::Container type_base;

  virtual void add(Gtk::Widget& first, Gtk::Widget& second, bool expand_second = false);
  virtual void add(Gtk::Widget& first, bool expand = false); //override
  void insert_before(Gtk::Widget& first, Gtk::Widget& second, Gtk::Widget& before, bool expand_second);
  void insert_before(Gtk::Widget& first, Gtk::Widget& before, bool expand);

  virtual void remove(Gtk::Widget& first); //override

  void set_columns_count(guint value);

  /** Sets the padding to put between the columns of widgets.
   */
  void set_column_padding(guint padding);
  
  /** Gets the padding between the columns of widgets.
   */
  guint get_column_padding() const;
  
  /** Sets the padding to put between the rows of widgets.
   */
  void set_row_padding(guint padding);
  
  /** Gets the padding between the rows of widgets.
   */
  guint get_row_padding() const;

  /** Show extra UI that is useful in RAD tools:
   */
  virtual void set_design_mode(bool value = true);

  void remove_all();

  // Implement forall which is not implemented in gtkmm:
  typedef sigc::slot<void, Widget&> ForallSlot;
  void forall(const ForallSlot& slot);

private:

  #ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  // These are the hand-coded C default signal handlers in case the
  // corresponding glibmm API has been disabled
  static void glom_size_request_impl(GtkWidget* widget, GtkRequisition* requisition);
  static void glom_size_allocate_impl(GtkWidget* widget, GtkAllocation* allocation);
  static void glom_add_impl(GtkContainer* container, GtkWidget* widget);
  static void glom_remove_impl(GtkContainer* container, GtkWidget* widget);

  static void glom_realize_impl(GtkWidget* widget);
  static void glom_unrealize_impl(GtkWidget* widget);
  static gboolean glom_expose_event_impl(GtkWidget* widget, GdkEventExpose* event);
  #endif
  //Overrides:

  //Handle child widgets:
  virtual void on_size_request(Gtk::Requisition* requisition);
  virtual void on_size_allocate(Gtk::Allocation& allocation);
  virtual GType child_type_vfunc() const;
  virtual void on_add(Gtk::Widget* child);
  virtual void forall_vfunc(gboolean include_internals, GtkCallback callback, gpointer callback_data);
  virtual void on_remove(Gtk::Widget* child);

  //Do extra drawing:
  //Virtual method overrides:
  void on_realize();
  void on_unrealize();
  bool on_expose_event(GdkEventExpose* event);

protected:
  int get_column_height(guint start_widget, guint widget_count, int& total_width) const;

  /** 
   * @result The height when the children are arranged optimally (so that the height is minimum).
   */
  int get_minimum_column_height(guint start_widget, guint columns_count, int& total_width) const;

  class FlowTableItem
  {
  public:
    FlowTableItem(Gtk::Widget* first, FlowTable* flowtable);
    FlowTableItem(Gtk::Widget* first, Gtk::Widget* second, FlowTable* flowtable);

    Gtk::Widget* m_first;
    Gtk::Widget* m_second;
    bool m_expand_first_full;
    bool m_expand_second;
    
    bool operator==(Gtk::Widget* child)
    {
      return (child == m_first || child == m_second);
    }
    
    //Cache the positions, so we can use them in on_expose_event:
    Gtk::Allocation m_first_allocation;
    Gtk::Allocation m_second_allocation;
  };

private:
  void insert_before(FlowTableItem& item, Gtk::Widget& before);

  int get_item_requested_height(const FlowTableItem& item) const;
  void get_item_requested_width(const FlowTableItem& item, int& first, int& second) const;
  void get_item_max_width_requested(guint start, guint height, guint& first_max_width, guint& second_max_width, guint& singles_max_width, bool& is_last_column) const; //TODO: maybe combine this with code in get_minimum_column_height().
  
  bool child_is_visible(const Gtk::Widget* widget) const;

  Gtk::Allocation assign_child(Gtk::Widget* widget, int x, int y);
  Gtk::Allocation assign_child(Gtk::Widget* widget, int x, int y, int width, int height);

protected:
  typedef std::vector<FlowTableItem> type_vecChildren;
  type_vecChildren m_children;
private:
  guint m_columns_count;
  guint m_column_padding, m_row_padding;
  bool m_design_mode;

  //Lines to draw in on_expose_event:
  typedef std::pair<Gdk::Point, Gdk::Point> type_line;
  typedef std::vector<type_line> type_vecLines;
  type_vecLines m_lines_vertical;
  type_vecLines m_lines_horizontal;

  //For drawing:
  Glib::RefPtr<Gdk::Window> m_refGdkWindow;
  Glib::RefPtr<Gdk::GC> m_refGC;
};

} //namespace Glom

#endif //GLOM_UTILITYWIDGETS_FLOWTABLE_H
