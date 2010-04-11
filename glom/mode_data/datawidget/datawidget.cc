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

#include "config.h"
#include "datawidget.h"
#include "entry.h"
#include "checkbutton.h"
#include "label.h"
#include <glom/mode_data/datawidget/comboentry.h>
#include <glom/mode_data/datawidget/combo.h>
#include <glom/mode_data/datawidget/combo_as_radio_buttons.h>
#include <glom/mode_data/datawidget/textview.h>
#include <glom/utility_widgets/imageglom.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/application.h>
#include <glom/mode_design/layout/dialog_choose_field.h>
#include <glom/mode_data/datawidget/dialog_choose_id.h>
#include <glom/mode_data/datawidget/dialog_choose_date.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h>
#include <glom/utils_ui.h>
#include <glom/glade_utils.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/button.h>
#endif //GLOM_ENABLE_MAEMO

#include <glibmm/i18n.h>

namespace Glom
{

static DataWidgetChildren::ComboChoices* create_combo_widget_for_field(const sharedptr<LayoutItem_Field>& field, const sharedptr<LayoutItem_Field>& layout_item_second = sharedptr<LayoutItem_Field>())
{
  DataWidgetChildren::ComboChoices* result = 0;
  bool as_radio_buttons = false; //TODO: Use this.
  const bool restricted = field->get_formatting_used().get_choices_restricted(as_radio_buttons);
  if(restricted)
  {
    if(as_radio_buttons)
      result = Gtk::manage(new DataWidgetChildren::ComboAsRadioButtons(layout_item_second));
    else
      result = Gtk::manage(new DataWidgetChildren::ComboGlom(layout_item_second));
  }
  else
    result = Gtk::manage(new DataWidgetChildren::ComboEntry(layout_item_second));

  return result;
}

DataWidget::DataWidget(const sharedptr<LayoutItem_Field>& field, const Glib::ustring& table_name, const Document* document)
:  m_child(0),
   m_button_go_to_details(0)
{
  const Field::glom_field_type glom_type = field->get_glom_type();
  set_layout_item(field, table_name);

  m_child = 0;
  LayoutWidgetField* pFieldWidget = 0;
  const Glib::ustring title = field->get_title_or_name();
  if(glom_type == Field::TYPE_BOOLEAN)
  {
    DataWidgetChildren::CheckButton* checkbutton = Gtk::manage( new DataWidgetChildren::CheckButton( title ) );
    checkbutton->show();
    checkbutton->signal_toggled().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

    //TODO: entry->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout );

    m_child = checkbutton;
    pFieldWidget = checkbutton;

    m_label.set_text( Glib::ustring() ); //It is not used.
  }
  else if(glom_type == Field::TYPE_IMAGE)
  {
    ImageGlom* image = Gtk::manage( new ImageGlom() );
    image->set_size_request(200, 200);
    //Gtk::Image* image = Gtk::manage( new Gtk::Image("/home/murrayc/gnome-small.jpg") );
    image->show();
    //TODO: Respond to double-click: checkbutton->signal_toggled().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

    //TODO: entry->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout );

    m_child = image;
    pFieldWidget = image;

    m_label.set_label(title);
    m_label.set_alignment(0);
    m_label.show();
  }
  // Use hildon widgets for date and time on maemo
  else
  {
    //The GNOME HIG says that labels should have ":" at the end:
    //http://library.gnome.org/devel/hig-book/stable/design-text-labels.html.en
    m_label.set_label(title + ':');
    m_label.set_alignment(0);
    m_label.show();

    //Use a Combo if there is a drop-down of choices (A "value list"), else an Entry:
    if(field->get_formatting_used().get_has_choices())
    {
      DataWidgetChildren::ComboChoices* combo = create_combo_widget_for_field(field);

      if(field->get_formatting_used().get_has_custom_choices())
      {
        //set_choices() needs this, for the numeric layout:
        combo->set_layout_item( get_layout_item(), table_name);

        combo->set_choices( field->get_formatting_used().get_choices_custom() );
      }
      else if(field->get_formatting_used().get_has_related_choices())
      {
        sharedptr<Relationship> choice_relationship;
        Glib::ustring choice_field, choice_second;
        field->get_formatting_used().get_choices(choice_relationship, choice_field, choice_second);
        if(choice_relationship && !choice_field.empty())
        {
          const Glib::ustring to_table = choice_relationship->get_to_table();

          const bool with_second = !choice_second.empty();

          if(with_second && document)
          {
            sharedptr<Field> field_second = document->get_field(to_table, choice_second);

            sharedptr<LayoutItem_Field> layout_field_second = sharedptr<LayoutItem_Field>::create();
            layout_field_second->set_full_field_details(field_second);
            //We use the default formatting for this field->

            combo = create_combo_widget_for_field(field, layout_field_second);
          }
          else
          {
            combo = create_combo_widget_for_field(field);
          }

          //set_choices() needs this, for the numeric layout:
          combo->set_layout_item( get_layout_item(), table_name);

          combo->set_choices_with_second( Utils::get_choice_values(field) );
        }
      }
      else
      {
        g_warning("DataWidget::DataWidget(): Unexpected choice type.");
      }

      pFieldWidget = combo;
    }
    else
    {
      if((glom_type == Field::TYPE_TEXT) && (field->get_formatting_used().get_text_format_multiline()))
      {
        DataWidgetChildren::TextView* textview = Gtk::manage(new DataWidgetChildren::TextView(glom_type));
        pFieldWidget = textview;
      }
      else  //TYPE_DATE, TYPE_NUMBER, etc.
      {
        DataWidgetChildren::Entry* entry = Gtk::manage(new DataWidgetChildren::Entry(glom_type));
        pFieldWidget = entry;
      }

      if(pFieldWidget)
        pFieldWidget->set_layout_item( get_layout_item(), table_name); //TODO_Performance: We only need this for the numerical format.
    }
  }

  if(pFieldWidget)
  {
    pFieldWidget->signal_edited().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

#ifndef GLOM_ENABLE_CLIENT_ONLY
    pFieldWidget->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout) );
    pFieldWidget->signal_user_requested_layout_properties().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout_properties) );
    pFieldWidget->signal_layout_item_added().connect( sigc::mem_fun(*this, &DataWidget::on_child_layout_item_added) );
#endif // !GLOM_ENABLE_CLIENT_ONLY


    m_child = dynamic_cast<Gtk::Widget*>(pFieldWidget);
    set_child_size_by_field(field);

    m_child->show_all();
  }

  if(m_child)
  {
    //Use the text formatting:
    apply_formatting(*m_child, field);
    
    bool child_added = false; //Don't use an extra container unless necessary.

    //Check whether the field controls a relationship,
    //meaning it identifies a record in another table.
    const bool field_used_in_relationship_to_one = document->get_field_used_in_relationship_to_one(table_name, field);
    //std::cout << "DEBUG: table_name=" << table_name << ", table_used=" << field->get_table_used(table_name) << ", field=" << field->get_name() << ", field_used_in_relationship_to_one=" << field_used_in_relationship_to_one << std::endl;

    //Check whether the field identifies a record in another table 
    //just because it is a primary key in that table:
    bool field_is_related_primary_key = false;
    if(document)
      field->set_full_field_details( document->get_field(field->get_table_used(table_name), field->get_name()) ); //Otherwise get_primary_key() returns false always.
    sharedptr<const Field> field_info = field->get_full_field_details();
    field_is_related_primary_key = 
      field->get_has_relationship_name() && 
      field_info && field_info->get_primary_key();
    //std::cout <<   "DEBUG: field->get_has_relationship_name()=" << field->get_has_relationship_name() << ", field_info->get_primary_key()=" <<  field_info->get_primary_key() << ", field_is_related_primary_key=" << field_is_related_primary_key << std::endl;


    Gtk::HBox* hbox_parent = 0; //Only used if there are extra widgets.

    const bool with_extra_widgets = field_used_in_relationship_to_one || field_is_related_primary_key || (glom_type == Field::TYPE_DATE);
    if(with_extra_widgets)
    {
      hbox_parent = Gtk::manage( new Gtk::HBox() ); //We put the child (and any extra stuff) in this:
      hbox_parent->set_spacing(Utils::DEFAULT_SPACING_SMALL);

      hbox_parent->pack_start(*m_child);
      hbox_parent->show();
      add(*hbox_parent);

      child_added = true;
    }

    if(glom_type == Field::TYPE_DATE)
    {
      //Let the user choose a date from a calendar dialog:
      #ifndef GLOM_ENABLE_MAEMO
      Gtk::Button* button_date = Gtk::manage(new Gtk::Button(_("..."))); //TODO: A better label/icon for "Choose Date".
      #else
      Gtk::Button* button_date = Gtk::manage(new Hildon::Button(Gtk::Hildon::SIZE_FINGER_HEIGHT, Hildon::BUTTON_ARRANGEMENT_HORIZONTAL, _("..."), ""));
      #endif
      button_date->set_tooltip_text(_("Choose a date from an on-screen calendar.")); 
      button_date->show();
      hbox_parent->pack_start(*button_date);
      button_date->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_choose_date));
    }

    if((field_used_in_relationship_to_one || field_is_related_primary_key) && hbox_parent)
    {
      //Add a button for related record navigation:
      #ifndef GLOM_ENABLE_MAEMO
      m_button_go_to_details = Gtk::manage(new Gtk::Button(Gtk::Stock::OPEN));
      #else
      m_button_go_to_details = Gtk::manage(new Hildon::Button(Gtk::Hildon::SIZE_FINGER_HEIGHT, Hildon::BUTTON_ARRANGEMENT_HORIZONTAL, _("Open"), ""));
      #endif
      m_button_go_to_details->set_tooltip_text(_("Open the record identified by this ID, in the other table."));
      hbox_parent->pack_start(*m_button_go_to_details);
      m_button_go_to_details->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_open_details));

      //Add a button to make it easier to choose an ID for this field.
      //Don't add this for simple related primary key fields, because they 
      //can generally not be edited via another table's layout.
      if(field_used_in_relationship_to_one)
      {
        #ifndef GLOM_ENABLE_MAEMO
        Gtk::Button* button_select = Gtk::manage(new Gtk::Button(Gtk::Stock::FIND));
        #else
        Gtk::Button* button_select = Gtk::manage(new Hildon::Button(Gtk::Hildon::SIZE_FINGER_HEIGHT, 
           Hildon::BUTTON_ARRANGEMENT_HORIZONTAL, _("Find"), ""));
        #endif
        button_select->set_tooltip_text(_("Enter search criteria to identify records in the other table, to choose an ID for this field."));
        hbox_parent->pack_start(*button_select);
        button_select->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_select_id));
      }
    }

    if(!child_added)
      add(*m_child);
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // GLOM_ENABLE_CLIENT_ONLY

  set_events(Gdk::BUTTON_PRESS_MASK);
  signal_style_changed().connect(sigc::mem_fun(*this, &DataWidget::on_self_style_changed));
}

DataWidget::~DataWidget()
{
}

void DataWidget::on_widget_edited()
{
  m_signal_edited.emit(get_value());
  update_go_to_details_button_sensitivity();
}

DataWidget::type_signal_edited DataWidget::signal_edited()
{
  return m_signal_edited;
}


void DataWidget::set_value(const Gnome::Gda::Value& value)
{
  Gtk::Widget* widget = get_data_child_widget();
  LayoutWidgetField* generic_field_widget = dynamic_cast<LayoutWidgetField*>(widget);
  if(generic_field_widget)
  {
    //if(generic_field_widget->get_layout_item())
    //  std::cout << "DataWidget::set_value(): generic_field_widget->get_layout_item()->get_name()=" << generic_field_widget->get_layout_item()->get_name() << std::endl;

    generic_field_widget->set_value(value);
  }
  else
  {
    Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(widget);
    if(checkbutton)
    {
      bool bValue = false;
      if(!value.is_null() && value.get_value_type() == G_TYPE_BOOLEAN)
        bValue = value.get_boolean();

      checkbutton->set_active( bValue );
    }
  }

  update_go_to_details_button_sensitivity();
}

void DataWidget::update_go_to_details_button_sensitivity()
{
  //If there is a Go-To-Details "Open" button, only enable it if there is 
  //an ID:
  if(m_button_go_to_details)
  {
    const Gnome::Gda::Value value = get_value();
    const bool enabled = !Conversions::value_is_empty(value);
    m_button_go_to_details->set_sensitive(enabled);
  }
}

Gnome::Gda::Value DataWidget::get_value() const
{
  const Gtk::Widget* widget = get_data_child_widget();
  const LayoutWidgetField* generic_field_widget = dynamic_cast<const LayoutWidgetField*>(widget);
  if(generic_field_widget)
    return generic_field_widget->get_value();
  else
  {
    const Gtk::CheckButton* checkbutton = dynamic_cast<const Gtk::CheckButton*>(widget);
    if(checkbutton)
    {
      return Gnome::Gda::Value(checkbutton->get_active());
    }
    else
      return Gnome::Gda::Value(); //null.
  }
}

Gtk::Label* DataWidget::get_label()
{
  return &m_label;
}

const Gtk::Label* DataWidget::get_label() const
{
  return &m_label;
}

void DataWidget::set_child_size_by_field(const sharedptr<const LayoutItem_Field>& field)
{
  const Field::glom_field_type glom_type = field->get_glom_type();
  int width = get_suitable_width(field);

  if(glom_type == Field::TYPE_IMAGE) //GtkImage widgets default to no size (invisible) if they are empty.
    m_child->set_size_request(width, width);
  else
  {
    int height = -1; //auto.
    if((glom_type == Field::TYPE_TEXT) && (field->get_formatting_used().get_text_format_multiline()))
    {
      #ifndef GLOM_ENABLE_MAEMO 
      int example_width = 0;
      int example_height = 0;
      Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout("example"); //TODO: Use different text, according to the current locale, or allow the user to choose an example?
      refLayout->get_pixel_size(example_width, example_height);
      
      if(example_height > 0)
        height = example_height * field->get_formatting_used().get_text_format_multiline_height_lines();
      #else
      //On Maemo, TextView widgets expand automatically.
      //TODO: Expansion only happens if both are -1, and vertical expansion never happens.
      //See bug https://bugs.maemo.org/show_bug.cgi?id=5515
      width = -1;
      height = -1; 
      #endif //GLOM_ENABLE_MAEMO
    }

    m_child->set_size_request(width, height);
  }
}

int DataWidget::get_suitable_width(const sharedptr<const LayoutItem_Field>& field_layout)
{
  return Utils::get_suitable_field_width_for_widget(*this, field_layout);
}

void DataWidget::set_viewable(bool viewable)
{
  Gtk::Widget* child = get_data_child_widget();
  Gtk::Entry* entry = dynamic_cast<Gtk::Entry*>(child);
  if(entry)
    entry->set_visibility(viewable); //TODO: This is not an ideal way to show non-viewable fields..
  else
  {
    Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(child);
    if(checkbutton)
      checkbutton->set_property("inconsistent", !viewable);
  }
}

void DataWidget::set_editable(bool editable)
{
  Gtk::Widget* child = get_data_child_widget();
  Gtk::Entry* entry = dynamic_cast<Gtk::Entry*>(child);
  if(entry)
    entry->set_editable(editable);
  else
  {
    LayoutWidgetBase* base = dynamic_cast<LayoutWidgetBase*>(child);
    if(base)
      base->set_read_only(!editable);
    else
    {
      Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(child);
      if(checkbutton)
        checkbutton->set_sensitive(editable);
    }
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool DataWidget::on_button_press_event(GdkEventButton *event)
{
  //g_warning("DataWidget::on_button_press_event_popup");

  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  Application* pApp = get_application();
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
      gdk_window_get_pointer( gtk_widget_get_window (Gtk::Widget::gobj()), 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }
  }

  return Gtk::EventBox::on_button_press_event(event);
}

sharedptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name)
{
  return offer_field_list(table_name, sharedptr<LayoutItem_Field>());
}

sharedptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& start_field)
{
  return offer_field_list(table_name, start_field, get_document(), get_application());
}

sharedptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& start_field, Document* document, Application* app)
{
  sharedptr<LayoutItem_Field> result;
  
  Dialog_ChooseField* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);

  if(dialog)
  {
    dialog->set_document(document, table_name, start_field);
    if(app)
      dialog->set_transient_for(*app);

    const int response = dialog->run();
    dialog->hide();
    if(response == Gtk::RESPONSE_OK)
    {
      //Get the chosen field:
      result = dialog->get_field_chosen();
    }

    delete dialog;
  }

  return result;
}

sharedptr<LayoutItem_Field> DataWidget::offer_field_layout(const sharedptr<const LayoutItem_Field>& start_field)
{
  sharedptr<LayoutItem_Field> result;

  Dialog_FieldLayout* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);

  add_view(dialog); //Give it access to the document.
  dialog->set_field(start_field, m_table_name);

  Gtk::Window* parent = get_application();
  if(parent)
    dialog->set_transient_for(*parent);

  const int response = dialog->run();
  dialog->hide();
  if(response == Gtk::RESPONSE_OK)
  {
    //Get the chosen field:
    result = dialog->get_field_chosen();
  }

  remove_view(dialog);
  delete dialog;

  return result;
}

void DataWidget::on_menupopup_activate_layout()
{
  //finish_editing();

  sharedptr<LayoutItem_Field> layoutField = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(layoutField)
  {
    sharedptr<LayoutItem_Field> itemchosen = offer_field_list(m_table_name, layoutField);
    if(itemchosen)
    {
      *layoutField = *itemchosen;
      signal_layout_changed().emit();
    }
  }
}

void DataWidget::on_menupopup_activate_layout_properties()
{
  //finish_editing();

  sharedptr<LayoutItem_Field> layoutField = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(layoutField)
  {
    sharedptr<LayoutItem_Field> itemchosen = offer_field_layout(layoutField);
    if(itemchosen)
    {
      *layoutField = *itemchosen;
      //set_layout_item(itemchosen, m_table_name);

      signal_layout_changed().emit();
    }
  }
}

void DataWidget::on_child_user_requested_layout_properties()
{
  on_menupopup_activate_layout_properties();
}

void DataWidget::on_child_user_requested_layout()
{
  on_menupopup_activate_layout();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Application* DataWidget::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<Application*>(pWindow);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void DataWidget::on_child_layout_item_added(LayoutWidgetBase::enumType item_type)
{
  //Tell the parent widget that this widget wants to add an item:
  signal_layout_item_added().emit(item_type);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Gtk::Widget* DataWidget::get_data_child_widget()
{
  return m_child;
}

const Gtk::Widget* DataWidget::get_data_child_widget() const
{
  return m_child;
}
 
 DataWidget::type_signal_open_details_requested DataWidget::signal_open_details_requested()
 {
   return m_signal_open_details_requested;
 }

 void DataWidget::on_button_open_details()
 {
   signal_open_details_requested().emit(get_value());
 }

 void DataWidget::on_button_select_id()
 {
   Gnome::Gda::Value chosen_id;
   bool chosen = offer_related_record_id_find(chosen_id);
   if(chosen)
   {
     set_value(chosen_id);
     m_signal_edited.emit(chosen_id);
   }
 }

void DataWidget::on_button_choose_date()
{
  DataWidgetChildren::Dialog_ChooseDate* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);

  if(dialog)
  {
    Gtk::Window* parent = get_application(); //This doesn't work (and would be wrong) when the widget is in a Field Definitions dialog.
    if(parent)
      dialog->set_transient_for(*parent);
    dialog->set_date_chosen(get_value());

    const int response = Glom::Utils::dialog_run_with_help(dialog);
    dialog->hide();
    if(response == Gtk::RESPONSE_OK)
    {
      //Get the chosen date
      const Gnome::Gda::Value value = dialog->get_date_chosen();
      set_value(value);

      m_signal_edited.emit(value);
    }

    delete dialog;
  }
}

void DataWidget::on_self_style_changed(const Glib::RefPtr<Gtk::Style>& /* style */)
{
  sharedptr<LayoutItem_Field> layoutField = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  set_child_size_by_field(layoutField);
}

bool DataWidget::offer_related_record_id_find(Gnome::Gda::Value& chosen_id)
{
  bool result = false;

  //Initialize output variable:
  chosen_id = Gnome::Gda::Value();

  DataWidgetChildren::Dialog_ChooseID* dialog = 0;
  Glom::Utils::get_glade_widget_derived_with_warning(dialog);

  if(dialog)
  {
    //dialog->set_document(get_document(), table_name, field);
    Gtk::Window* parent = get_application();
    if(parent)
      dialog->set_transient_for(*parent);
    add_view(dialog);

    //Discover the related table, in the relationship that uses this ID field:
    Glib::ustring related_table_name;
    sharedptr<const LayoutItem_Field> layoutField = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
    if(layoutField)
    {
      sharedptr<const Relationship> relationship = get_document()->get_field_used_in_relationship_to_one(m_table_name, layoutField);
      if(relationship)
        related_table_name = relationship->get_to_table();
    }
    else
      g_warning("get_layout_item() was not a LayoutItem_Field");

    dialog->init_db_details(related_table_name, Base_DB::get_active_layout_platform(get_document()));


    const int response = dialog->run();
    dialog->hide();
    if(response == Gtk::RESPONSE_OK)
    {
      //Get the chosen field:
      result = dialog->get_id_chosen(chosen_id);
    }

    remove_view(dialog);
    delete dialog;
  }

  return result;
}

} //namespace Glom