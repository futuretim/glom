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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "dialog_existing_or_new.h"

#include <libxml++/parsers/saxparser.h>
#include <libxml/parser.h> //For xmlStopParser()

#include <glibmm/i18n.h>
#include <giomm/contenttype.h>
#include <giomm/resource.h>
#include <gtkmm/recentmanager.h>
#include <gtkmm/filechooserdialog.h>
#include <glibmm/miscutils.h>
#include <glibmm/vectorutils.h>

#ifdef G_OS_WIN32
#include <glibmm/fileutils.h>
#include <glib.h>
#else
#include <libepc/service-type.h>
#endif

#include <iostream>

#ifdef GLOM_ENABLE_CLIENT_ONLY
static const int NEW_PAGE = 1;
#endif /* GLOM_ENABLE_CLIENT_ONLY */

namespace
{

const char* NETWORK_DUMMY_TEXT = N_("No sessions found on the local network.");


//TODO_Performance: A DomParser or XmlReader might be faster, or even a regex.
/// Reads the title of an example from the first few characters of the XML.
class Parser: public xmlpp::SaxParser
{
public:
  Parser()
  {}

  Glib::ustring get_example_title(const std::string& beginning)
  {
    try
    {
      parse_chunk(beginning);
    }
    catch(const xmlpp::exception& /* ex */)
    {
      //Ignore these, returning as much of the title as we have managed to retrieve.
      //Recent versions of libxml now cause an exception after we call xmlStopParser() anyway.
      //TODO: How can we get the error code from the exception anyway?
    }

    return m_title;
  }

private:
  void on_start_element(const Glib::ustring& name, const AttributeList& attributes) override
  {
    if(m_title.empty()) // Already found name? Wait for parse_chunk() call to return.
    {
      if(name == "glom_document") //See document_glom.cc for the #defines.
      {
        for(const auto& item : attributes)
        {
          if(item.name == "database_title")
          {
            m_title = item.value;

            // Stop parsing here because we have what we need:
            xmlStopParser(context_);

            break;
          }
        }
      }
    }
  }

  Glib::ustring m_title;
};

} //anonymous namespace

namespace Glom
{

const char Dialog_ExistingOrNew::glade_id[] = "dialog_existing_or_new";
const bool Dialog_ExistingOrNew::glade_developer(false);

Dialog_ExistingOrNew::Dialog_ExistingOrNew(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
#ifdef GLOM_ENABLE_CLIENT_ONLY
  //Don't mention creation of new documents in client-only mode:
  Gtk::Label* label = nullptr;
  builder->get_widget("existing_or_new_label", label);
  label->set_text(_("Open a Document"));
#endif //GLOM_ENABLE_CLIENT_ONLY

  builder->get_widget("existing_or_new_existing_treeview", m_existing_view);
  g_assert(m_existing_view);

  builder->get_widget("existing_or_new_notebook", m_notebook);
  builder->get_widget("existing_or_new_button_select", m_select_button);

  if(!m_notebook || !m_select_button)
    throw std::runtime_error("Glade file does not contain the notebook or the select button for ExistingOrNew dialog.");

  m_existing_model = Gtk::TreeStore::create(m_existing_columns);
  m_existing_model->set_sort_column(m_existing_columns.m_col_time, Gtk::SORT_DESCENDING);
  m_existing_view->set_model(m_existing_model);



  m_iter_existing_other = m_existing_model->append();
  (*m_iter_existing_other)[m_existing_columns.m_col_title] = _("Select File");

#ifndef G_OS_WIN32
  m_iter_existing_network = m_existing_model->append();
  (*m_iter_existing_network)[m_existing_columns.m_col_title] = _("Local Network");
#endif

  m_iter_existing_recent = m_existing_model->append();
  (*m_iter_existing_recent)[m_existing_columns.m_col_title] = _("Recently Opened");



  m_existing_column_title.set_expand(true);
  m_existing_column_title.pack_start(m_existing_icon_renderer, false);
  m_existing_column_title.pack_start(m_existing_title_renderer, true);
  m_existing_column_title.set_cell_data_func(m_existing_icon_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::existing_icon_data_func));
  m_existing_column_title.set_cell_data_func(m_existing_title_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::existing_title_data_func));
  m_existing_view->append_column(m_existing_column_title);

  m_existing_view->set_headers_visible(false);
  m_existing_view->signal_row_activated().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_row_activated));


  // Browse local network
#ifndef G_OS_WIN32
  gchar* service_type = epc_service_type_new(EPC_PROTOCOL_HTTPS, "glom");
  m_service_monitor = epc_service_monitor_new_for_types(nullptr, service_type, (void*)nullptr);
  g_signal_connect(m_service_monitor, "service-found", G_CALLBACK(on_service_found_static), this);
  g_signal_connect(m_service_monitor, "service-removed", G_CALLBACK(on_service_removed_static), this);
  g_free(service_type);
#endif

  // Add recently used files
  const auto infos = Gtk::RecentManager::get_default()->get_items();
  for(const auto& info : infos)
  {
    if(info->get_mime_type() == "application/x-glom")
    {
      auto iter = m_existing_model->append(m_iter_existing_recent->children());
      (*iter)[m_existing_columns.m_col_title] = info->get_display_name();
      (*iter)[m_existing_columns.m_col_time] = info->get_modified();
      (*iter)[m_existing_columns.m_col_recent_info] = info;
    }
  }

  const auto children = m_iter_existing_recent->children();
  if(children.begin() == children.end())
    m_iter_existing_recent_dummy = create_dummy_item_existing(m_iter_existing_recent, _("No recently used documents available."));

#ifndef G_OS_WIN32
  // Will be removed when items are added:
  m_iter_existing_network_dummy = create_dummy_item_existing(m_iter_existing_network, _(NETWORK_DUMMY_TEXT));
#endif


  // Expand recently used files and the networked files,
  // because the contents help to explain what this is:
  m_existing_view->expand_row(m_existing_model->get_path(m_iter_existing_recent), false);
#ifndef G_OS_WIN32
  m_existing_view->expand_row(m_existing_model->get_path(m_iter_existing_network), false);
#endif

  m_select_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_select_clicked));

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_notebook->signal_switch_page().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_switch_page));
#endif /* !GLOM_ENABLE_CLIENT_ONLY */

  auto existing_view_selection = m_existing_view->get_selection();
  existing_view_selection->signal_changed().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_selection_changed));
  existing_view_selection->set_select_function( sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_select_func) );

#ifndef GLOM_ENABLE_CLIENT_ONLY
  builder->get_widget("existing_or_new_new_treeview", m_new_view);
  g_assert(m_new_view);
  m_new_model = Gtk::TreeStore::create(m_new_columns);
  m_new_view->set_model(m_new_model);

  m_iter_new_empty = m_new_model->append();
  (*m_iter_new_empty)[m_new_columns.m_col_title] = _("New Empty Document");

  m_iter_new_template = m_new_model->append();
  (*m_iter_new_template)[m_new_columns.m_col_title] = _("New From Template");

  m_new_column_title.set_expand(true);
  m_new_column_title.pack_start(m_new_icon_renderer, false);
  m_new_column_title.pack_start(m_new_title_renderer, true);
  m_new_column_title.set_cell_data_func(m_new_icon_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::new_icon_data_func));
  m_new_column_title.set_cell_data_func(m_new_title_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::new_title_data_func));
  m_new_view->append_column(m_new_column_title);

  m_new_view->set_headers_visible(false);
  m_new_view->signal_row_activated().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_row_activated));

  m_iter_new_template_dummy = create_dummy_item_new(m_iter_new_template, "No templates available.");

  auto new_view_selection = m_new_view->get_selection();
  new_view_selection->signal_changed().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_selection_changed));
  new_view_selection->set_select_function( sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_select_func) );
#else /* GLOM_ENABLE_CLIENT_ONLY */
  m_notebook->remove_page(NEW_PAGE);
  m_notebook->set_show_tabs(false);
#endif /* !GLOM_ENABLE_CLIENT_ONLY */


 // Load example files:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Show the bundled (in a GResource) example files,
  list_examples();
#endif //!GLOM_ENABLE_CLIENT_ONLY

  //Make sure the first item is visible:
  m_existing_view->scroll_to_row(
    Gtk::TreeModel::Path(m_iter_existing_other) );

  update_ui_sensitivity();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Dialog_ExistingOrNew::list_examples()
{
  //std::cout << "debug: " << G_STRFUNC << ": path=" << path << std::endl;

  try
  {
    const char* examples_dir = "/org/gnome/glom/examples/";
    const auto examples = Gio::Resource::enumerate_children_global(examples_dir);

    bool example_found = false;
    for(const auto& example_name : examples)
    {
      const auto full_path = Glib::build_filename(examples_dir, example_name);
      const auto title = get_title_from_example(full_path);
      if(!title.empty())
      {
        append_example(title, full_path);
      	example_found = true;
      }
    }

    return example_found;
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << G_STRFUNC << ": Could not enumerate examples. Error=" << ex.what() << std::endl;
  }

  return false;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Dialog_ExistingOrNew::~Dialog_ExistingOrNew()
{
#ifndef G_OS_WIN32
  if(m_service_monitor)
  {
    g_object_unref(m_service_monitor);
    m_service_monitor = nullptr;
  }

  // Release the service infos in the treestore
  if(!m_iter_existing_network_dummy.get())
  {
    const auto children = m_iter_existing_network->children();
    for(const auto& item : children)
      epc_service_info_unref(item[m_existing_columns.m_col_service_info]);
  }
#endif

}

bool Dialog_ExistingOrNew::on_existing_select_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool /* path_currently_selected */)
{
  auto iter = model->get_iter(path);
#ifndef G_OS_WIN32
  if(iter == m_iter_existing_network)
    return false; /* Do not allow parent nodes to be selected. */
#endif
  if(iter == m_iter_existing_recent)
    return false;

  return true;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Dialog_ExistingOrNew::on_new_select_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool /* path_currently_selected */)
{
  auto iter = model->get_iter(path);
  if(iter == m_iter_new_template)
    return false; /* Do not allow parent nodes to be selected. */
  else
    return true;
}
#endif //GLOM_ENABLE_CLIENT_ONLY

Dialog_ExistingOrNew::Action Dialog_ExistingOrNew::get_action_impl(Gtk::TreeModel::iterator& iter) const
{
  if(m_notebook->get_current_page() == 0)
  {
    if(m_existing_view->get_selection()->count_selected_rows() == 0)
      return Action::NONE;

    iter = m_existing_view->get_selection()->get_selected();
    if(m_existing_model->is_ancestor(m_iter_existing_recent, iter))
      return Action::OPEN_URI;
#ifndef G_OS_WIN32
    if(m_existing_model->is_ancestor(m_iter_existing_network, iter))
      return Action::OPEN_REMOTE;
#endif
    if(iter == m_iter_existing_other)
      return Action::OPEN_URI;

    return Action::NONE;
  }
  else
  {
    #ifndef GLOM_ENABLE_CLIENT_ONLY
    if(m_new_view->get_selection()->count_selected_rows() == 0)
      return Action::NONE;

    iter = m_new_view->get_selection()->get_selected();
    if(m_new_model->is_ancestor(m_iter_new_template, iter))
      return Action::NEW_FROM_TEMPLATE;
    else if(iter == m_iter_new_empty)
      return Action::NEW_EMPTY;
    else
      return Action::NONE;
    #endif //GLOM_ENABLE_CLIENT_ONLY
  }

  return Action::NONE;
}

Dialog_ExistingOrNew::Action Dialog_ExistingOrNew::get_action() const
{
  Gtk::TreeModel::iterator iter;
  return get_action_impl(iter);
}

Glib::ustring Dialog_ExistingOrNew::get_uri() const
{
  Gtk::TreeModel::iterator iter;
  const auto action = get_action_impl(iter);

  #ifndef GLOM_ENABLE_CLIENT_ONLY
  if(action == Action::NEW_FROM_TEMPLATE)
  {
    return (*iter)[m_new_columns.m_col_template_uri];
  }
  else
  #endif //GLOM_ENABLE_CLIENT_ONLY
  if(action == Action::OPEN_URI)
  {
    if(iter == m_iter_existing_other)
    {
      return m_chosen_uri;
    }
    else
    {
      Glib::RefPtr<Gtk::RecentInfo> info = (*iter)[m_existing_columns.m_col_recent_info];
      return info->get_uri();
    }
  }
  else
  {
    throw std::logic_error("Dialog_ExistingOrNew::get_uri: action is neither NEW_FROM_TEMPLATE nor OPEN_URI");
  }
}

#ifndef G_OS_WIN32
EpcServiceInfo* Dialog_ExistingOrNew::get_service_info() const
{
  Gtk::TreeModel::iterator iter;
  Action action = get_action_impl(iter);

  if(action == Action::OPEN_REMOTE)
    return (*iter)[m_existing_columns.m_col_service_info];
  else
    throw std::logic_error("Dialog_ExistingOrNew::get_service_info: action is not OPEN_REMOTE");

  return nullptr;
}

Glib::ustring Dialog_ExistingOrNew::get_service_name() const
{
  Gtk::TreeModel::iterator iter;
  Action action = get_action_impl(iter);

  if(action == Action::OPEN_REMOTE)
    return (*iter)[m_existing_columns.m_col_service_name];
  else
    throw std::logic_error("Dialog_ExistingOrNew::get_service_name: action is not OPEN_REMOTE");
}
#endif

std::shared_ptr<Gtk::TreeModel::iterator> Dialog_ExistingOrNew::create_dummy_item_existing(const Gtk::TreeModel::iterator& parent, const Glib::ustring& text)
{
  auto iter = m_existing_model->append(parent->children());
  (*iter)[m_existing_columns.m_col_title] = text;
  return std::make_shared<Gtk::TreeModel::iterator>(iter);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
std::shared_ptr<Gtk::TreeModel::iterator> Dialog_ExistingOrNew::create_dummy_item_new(const Gtk::TreeModel::iterator& parent, const Glib::ustring& text)
{
  auto iter = m_new_model->append(parent->children());
  (*iter)[m_new_columns.m_col_title] = text;
  return std::make_shared<Gtk::TreeModel::iterator>(iter);
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::existing_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  auto pixbuf_renderer = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
  if(!pixbuf_renderer)
    throw std::logic_error("Renderer not a pixbuf renderer in existing_icon_data_func");

  pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_BUTTON;
  pixbuf_renderer->property_icon_name() = "";
  pixbuf_renderer->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>();

  if(iter == m_iter_existing_recent)
    pixbuf_renderer->property_icon_name() = "folder";

  pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_BUTTON;
  pixbuf_renderer->property_icon_name() = std::string();
  pixbuf_renderer->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>();

  if(iter == m_iter_existing_recent)
    pixbuf_renderer->property_icon_name() = "folder";
#ifndef G_OS_WIN32
  else if(iter == m_iter_existing_network)
    pixbuf_renderer->property_icon_name() = "folder";
#endif
  else if(iter == m_iter_existing_other)
    pixbuf_renderer->property_icon_name() = "folder";
  else if(m_iter_existing_recent_dummy.get() && iter == *m_iter_existing_recent_dummy)
    pixbuf_renderer->property_icon_name() = std::string(); // TODO: Use Stock::STOP instead?
#ifndef G_OS_WIN32
  else if(m_iter_existing_network_dummy.get() && iter == *m_iter_existing_network_dummy)
    pixbuf_renderer->property_icon_name() = std::string(); // TODO: Use Stock::STOP instead?
#endif
  else
  {
    if(m_existing_model->is_ancestor(m_iter_existing_recent, iter))
    {
      //Glib::RefPtr<Gtk::RecentInfo> info = (*iter)[m_existing_columns.m_col_recent_info];
      //pixbuf_renderer->property_pixbuf() = (*info)->get_icon(Gtk::ICON_SIZE_BUTTON);
      pixbuf_renderer->property_icon_name() = Glib::ustring("glom");
    }
#ifndef G_OS_WIN32
    else if(m_existing_model->is_ancestor(m_iter_existing_network, iter))
    {
      //pixbuf_renderer->property_icon_name() = Gtk::Stock::CONNECT.id;
      pixbuf_renderer->property_icon_name() = Glib::ustring("glom");
    }
#endif
    else
    {
      throw std::logic_error("Unexpected iterator in existing_icon_data_func");
    }
  }
}

void Dialog_ExistingOrNew::existing_title_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  auto text_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!text_renderer)
    throw std::logic_error("Renderer not a text renderer in existing_title_data_func");

  text_renderer->property_text() = (*iter)[m_existing_columns.m_col_title];

  // Default: Use default color:
  text_renderer->property_foreground_set() = false;

  // Use grey if parent item has no children:
#ifndef G_OS_WIN32
  if( (iter == m_iter_existing_network && m_iter_existing_network_dummy.get()) ||
      (iter == m_iter_existing_recent && m_iter_existing_recent_dummy.get()))
#else
  if(iter == m_iter_existing_recent && m_iter_existing_recent_dummy.get())
#endif
  {
    text_renderer->property_foreground() = "grey";
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Dialog_ExistingOrNew::new_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  auto pixbuf_renderer = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
  if(!pixbuf_renderer)
    throw std::logic_error("Renderer not a pixbuf renderer in new_icon_data_func");

  pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_BUTTON;
  pixbuf_renderer->property_icon_name() = "";
  pixbuf_renderer->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>();

  if(iter == m_iter_new_empty)
    pixbuf_renderer->property_icon_name() = "folder";
  else if(iter == m_iter_new_template)
    pixbuf_renderer->property_icon_name() = "folder"; // TODO: More meaningful icon?
  else if(m_iter_new_template_dummy.get() && iter == *m_iter_new_template_dummy)
    pixbuf_renderer->property_icon_name() = "dialog-error"; // TODO: Use Stock::STOP instead?
  else
  {
    if(m_new_model->is_ancestor(m_iter_new_template, iter))
    {
      pixbuf_renderer->property_icon_name() = Glib::ustring("glom");
    }
    else
    {
      throw std::logic_error("Unexpected iterator in new_icon_data_func");
    }
  }
}

void Dialog_ExistingOrNew::new_title_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  auto text_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!text_renderer)
    throw std::logic_error("Renderer not a text renderer in new_title_data_func");

  text_renderer->property_text() = (*iter)[m_new_columns.m_col_title];

  // Default: Use default color
  text_renderer->property_foreground_set() = false;
  // Use grey if parent item has no children
  if( (iter == m_iter_new_template && m_iter_new_template_dummy.get()))
  {
    text_renderer->property_foreground() = "grey";
  }
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::on_switch_page(Gtk::Widget* /* page */, guint /* page_num */)
{
  update_ui_sensitivity();
}

void Dialog_ExistingOrNew::on_existing_selection_changed()
{
  update_ui_sensitivity();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Dialog_ExistingOrNew::on_new_selection_changed()
{
  update_ui_sensitivity();
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::update_ui_sensitivity()
{
  bool sensitivity = false;

  if(m_notebook->get_current_page() == 0)
  {
    const auto count = m_existing_view->get_selection()->count_selected_rows();

    if(count == 0)
    {
      sensitivity = false;
    }
    else
    {
      auto sel = m_existing_view->get_selection()->get_selected();
      sensitivity = (sel != m_iter_existing_recent);
#ifndef G_OS_WIN32
      sensitivity = sensitivity && (sel != m_iter_existing_network);
#endif

      sensitivity = sensitivity && (!m_iter_existing_recent_dummy.get() || sel != *m_iter_existing_recent_dummy);
#ifndef G_OS_WIN32
      sensitivity = sensitivity && (!m_iter_existing_network_dummy.get() || sel != *m_iter_existing_network_dummy);
#endif
    }
  }
#ifndef GLOM_ENABLE_CLIENT_ONLY
  else
  {
    const auto count = m_new_view->get_selection()->count_selected_rows();

    if(count == 0)
    {
      sensitivity = false;
    }
    else
    {
      auto sel = m_new_view->get_selection()->get_selected();
      sensitivity = (sel != m_iter_new_template &&
                     (!m_iter_new_template_dummy.get() || sel != *m_iter_new_template_dummy));
    }
  }
#endif //GLOM_ENABLE_CLIENT_ONLY

  m_select_button->set_sensitive(sensitivity);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
Glib::ustring Dialog_ExistingOrNew::get_title_from_example(const std::string& resource_name)
{
  try
  {
    auto stream =
      Gio::Resource::open_stream_global(resource_name);

    //TODO: Really do this asynchronously?
    m_current_buffer.reset(new buffer);
    const auto bytes_read = stream->read(m_current_buffer->buf, buffer::SIZE);
    const std::string data(m_current_buffer->buf, bytes_read);
    // TODO: Check that data is valid UTF-8, the last character might be truncated

    Parser parser;
    return (Glib::ustring(parser.get_example_title(data)));
  }
  catch(const Glib::Exception& exception)
  {
    std::cerr << G_STRFUNC << ": Could not enumerate files in examples directory: " << exception.what() << std::endl;

    m_current_buffer.reset();
  }

  // File is not a glom file, continue with next.
  return Glib::ustring();
}

void Dialog_ExistingOrNew::append_example(const Glib::ustring& title, const std::string& resource_name)
{
  if(!m_new_model)
  {
    std::cerr << G_STRFUNC << ": m_new_model is null\n";
    return;
  }

  if(!m_iter_new_template)
  {
    std::cerr << G_STRFUNC << ": m_iter_new_template is null\n";
    return;
  }

  try
  {
    const auto is_first_item = m_iter_new_template_dummy.get();

    // Add to list.
    auto iter = m_new_model->append(m_iter_new_template->children());
    (*iter)[m_new_columns.m_col_title] = title;
    (*iter)[m_new_columns.m_col_template_uri] = "resource://" + resource_name; //GFile understands this for actual files, though not directories.

    if(is_first_item)
    {
      // Remove dummy.
      m_new_model->erase(*m_iter_new_template_dummy);
      m_iter_new_template_dummy.reset();
      // Expand if this is the first item.
      m_new_view->expand_row(m_new_model->get_path(m_iter_new_template), false);
    }
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << G_STRFUNC << ": Could not read example: " << resource_name << ": " << ex.what() << std::endl;
  }
}
#endif /* !GLOM_ENABLE_CLIENT_ONLY */

#ifndef G_OS_WIN32
void Dialog_ExistingOrNew::on_service_found_static(EpcServiceMonitor* /* monitor */, gchar* name, EpcServiceInfo* info, gpointer user_data)
{
  static_cast<Dialog_ExistingOrNew*>(user_data)->on_service_found(name, info);
}

void Dialog_ExistingOrNew::on_service_removed_static(EpcServiceMonitor* /* monitor */, gchar* name, gchar* type, gpointer user_data)
{
  static_cast<Dialog_ExistingOrNew*>(user_data)->on_service_removed(name, type);
}

void Dialog_ExistingOrNew::on_service_found(const Glib::ustring& name, EpcServiceInfo* info)
{
  //Translator hint: This is <Service Name> on <Host> (via Network Interface such as eth0).
  gchar* title = g_strdup_printf(_("%s on %s (via %s)"), name.c_str(), epc_service_info_get_host(info), epc_service_info_get_interface(info));
  auto iter = m_existing_model->prepend(m_iter_existing_network->children());
  (*iter)[m_existing_columns.m_col_title] = title;
  (*iter)[m_existing_columns.m_col_time] = std::time(nullptr); /* sort more recently discovered items above */
  (*iter)[m_existing_columns.m_col_service_name] = name;
  (*iter)[m_existing_columns.m_col_service_info] = info;

  epc_service_info_ref(info);
  g_free(title);

  // Remove dummy item
  if(m_iter_existing_network_dummy.get())
  {
    m_existing_model->erase(*m_iter_existing_network_dummy);
    m_iter_existing_network_dummy.reset();
  }
}

void Dialog_ExistingOrNew::on_service_removed(const Glib::ustring& name, const Glib::ustring& /* type */)
{
  // Find the entry with the given name
  const auto children = m_iter_existing_network->children();
  for(auto iter = children.begin(); iter != children.end(); ++ iter)
  {
    if((*iter)[m_existing_columns.m_col_service_name] == name)
    {
      const auto was_expanded = m_existing_view->row_expanded(m_existing_model->get_path(iter));

      // Remove from store
      epc_service_info_unref((*iter)[m_existing_columns.m_col_service_info]);
      m_existing_model->erase(iter);

      // Reinsert dummy, if necessary
      if(children.begin() == children.end())
        m_iter_existing_network_dummy = create_dummy_item_existing(m_iter_existing_network, _(NETWORK_DUMMY_TEXT));

      if(was_expanded)
        m_existing_view->expand_row(m_existing_model->get_path(iter), false);

      break;
    }
  }
}
#endif // !G_OS_WIN32

void Dialog_ExistingOrNew::on_existing_row_activated(const Gtk::TreeModel::Path& /* path */, Gtk::TreeViewColumn* /* column */)
{
  if(m_select_button->is_sensitive())
    on_select_clicked();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Dialog_ExistingOrNew::on_new_row_activated(const Gtk::TreeModel::Path& /* path */, Gtk::TreeViewColumn* /* column */)
{
  if(m_select_button->is_sensitive())
    on_select_clicked();
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::on_select_clicked()
{
  Gtk::TreeModel::iterator iter;
  Action action = get_action_impl(iter);

  if(action == Action::OPEN_URI && iter == m_iter_existing_other)
  {
    Gtk::FileChooserDialog dialog(*this, "Open Glom File");
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Open"), Gtk::RESPONSE_OK);
    dialog.set_default_response(Gtk::RESPONSE_OK);

    auto filter = Gtk::FileFilter::create();
    filter->add_mime_type("application/x-glom");
    filter->set_name("Glom files");
    dialog.add_filter(filter);

    const auto response_id = dialog.run();
    if(response_id == Gtk::RESPONSE_OK)
    {
      m_chosen_uri = dialog.get_uri();
      response(Gtk::RESPONSE_ACCEPT);
    }
  }
  else
  {
    response(Gtk::RESPONSE_ACCEPT);
  }
}


} //namespace Glom
