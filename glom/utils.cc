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

#include "utils.h"
#include "connectionpool.h"
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>
#include <libgnomevfsmm.h>
#include <libgnome/gnome-url.h>
#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>


Glib::ustring GlomUtils::trim_whitespace(const Glib::ustring& text)
{
  //TODO_Performance:

  Glib::ustring result = text;

  //Find non-whitespace from front:
  Glib::ustring::size_type posFront = Glib::ustring::npos;
  Glib::ustring::size_type pos = 0;
  for(Glib::ustring::iterator iter = result.begin(); iter != result.end(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
    {
      posFront = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(posFront);


 //Find non-whitespace from back:
  Glib::ustring::size_type posBack = Glib::ustring::npos;
  pos = 0;
  for(Glib::ustring::reverse_iterator iter = result.rbegin(); iter != result.rend(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
    {
      posBack = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(0, result.size() - posBack);

  return result;
}



Glib::ustring GlomUtils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause, const Glib::ustring& sort_clause)
{
  Glib::ustring result;

  Glib::ustring sql_part_fields;

  typedef std::list<Relationship> type_list_relationships;
  type_list_relationships list_relationships;

  for(type_vecLayoutFields::const_iterator iter =  fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    if(iter != fieldsToGet.begin())
      sql_part_fields += ", ";


    if(!iter->get_has_relationship_name())
    {
      sql_part_fields += ( table_name + "." );
    }
    else
    {
      Glib::ustring relationship_name = iter->get_relationship_name();
      if(!relationship_name.empty())
      {
        const Relationship relationship = iter->m_relationship;

        const Glib::ustring field_table_name = relationship.get_to_table();
        if(field_table_name.empty())
        {
          g_warning("build_sql_select_with_where_clause(): field_table_name is null. relationship name = %s", relationship.get_name().c_str());
        }

        sql_part_fields += ( field_table_name + "." );

        //Add the relationship to the list:
        type_list_relationships::const_iterator iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_FieldHasName<Relationship>( relationship_name ) );
        if(iterFind == list_relationships.end()) //If the table is not yet in the list:
          list_relationships.push_back(relationship);
      }
    }

    sql_part_fields += iter->get_name();
  }

  result =  "SELECT " + sql_part_fields +
    " FROM " + table_name;

  //LEFT OUTER JOIN will get the field values from the other tables, and give us our fields for this table even if there is no corresponding value in the other table.
  Glib::ustring sql_part_leftouterjoin; 
  for(type_list_relationships::const_iterator iter = list_relationships.begin(); iter != list_relationships.end(); ++iter)
  {
    const Relationship& relationship = *iter;
    sql_part_leftouterjoin += " LEFT OUTER JOIN " + relationship.get_to_table() +
      " ON (" + relationship.get_from_table() + "." + relationship.get_from_field() + " = " +
      relationship.get_to_table() + "." + relationship.get_to_field() +
      ")";
  }

  result += sql_part_leftouterjoin;

  if(!where_clause.empty())
    result += " WHERE " + where_clause;

 if(!sort_clause.empty())
    result += " ORDER BY " + sort_clause;

  return result;
}


GlomUtils::type_list_values_with_second GlomUtils::get_choice_values(const LayoutItem_Field& field)
{
  type_list_values_with_second list_values;

  Glib::ustring choice_relationship_name, choice_field, choice_second;
  field.get_choices(choice_relationship_name, choice_field, choice_second);

  const Relationship relationship = field.m_choices_related_relationship;
  const Glib::ustring to_table = relationship.get_to_table();
  if(to_table.empty())
  {
    g_warning("get_choice_values(): table_name is null. relationship name = %s", field.m_choices_related_relationship.get_name().c_str());
    return list_values;
  }

  const bool with_second = !choice_second.empty();
  const Glib::ustring sql_second = to_table + "." + choice_second;

  //Get possible values from database, sorted by the first column.
  Glib::ustring sql_query = "SELECT " + to_table + "." + choice_field;
  if(with_second)
    sql_query += ", " + sql_second;

  sql_query += " FROM " + relationship.get_to_table() + " ORDER BY " + to_table + "." + choice_field;

  //Connect to database:
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect();


  std::cout << "get_choice_values: Executing SQL: " << sql_query << std::endl;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->execute_single_command(sql_query);

  if(datamodel)
  {
    guint count = datamodel->get_n_rows();
    for(guint row = 0; row < count; ++row)
    {
      std::pair<Gnome::Gda::Value, Gnome::Gda::Value> itempair;
      itempair.first = datamodel->get_value_at(0, row);

      if(with_second)
        itempair.second = datamodel->get_value_at(1, row);

      list_values.push_back(itempair);
    }
  }

  return list_values;
}


void GlomUtils::transform_and_open(const xmlpp::Document& xml_document, const Glib::ustring& xsl_file_path)
{
  //Use libxslt to convert the XML to HTML:
  Glib::ustring result = xslt_process(xml_document, GLOM_XSLTDIR + xsl_file_path);
  std::cout << "After xslt: " << result << std::endl;

  //Save it to a temporary file and show it in a browser:
  //TODO: This actually shows it in gedit.
  const Glib::ustring temp_uri = "file:///tmp/glom_printout.html";
  std::cout << "temp_uri=" << temp_uri << std::endl;

  Gnome::Vfs::Handle write_handle;

  try
  {
    //0660 means "this user and his group can read and write this non-executable file".
    //The 0 prefix means that this is octal.
    write_handle.create(temp_uri, Gnome::Vfs::OPEN_WRITE, false, 0660 /* leading zero means octal */);
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    // If the operation was not successful, print the error and abort
    return; // print_error(ex, output_uri_string);
  }

  try
  {
    //Write the data to the output uri
    /* GnomeVFSFileSize bytes_written = */ write_handle.write(result.data(), result.bytes());
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    // If the operation was not successful, print the error and abort
    return; //print_error(ex, output_uri_string);
  }

  //Use the GNOME browser:
  GError* error = 0;
  gnome_url_show(temp_uri.c_str(), &error); //This is in libgnome.
}

Glib::ustring GlomUtils::xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt)
{
  Glib::ustring  result;

  //Use libxslt to transform the XML:
  xmlDocPtr style = xmlReadFile(filepath_xslt.c_str(), 0, 0);
  if(style)
  {
    //Parse the stylesheet:
    xsltStylesheetPtr cur = xsltParseStylesheetDoc(style);
    if(cur)
    {
      //Use the parsed stylesheet on the XML:
      xmlDocPtr pDocOutput = xsltApplyStylesheet(cur, const_cast<xmlDoc*>(xml_document.cobj()), 0);
      xsltFreeStylesheet(cur);

      //Get the output text:
      xmlChar* buffer = 0;
      int length = 0;
      xmlDocDumpFormatMemoryEnc(pDocOutput, &buffer, &length, 0, 0);

      if(buffer)
      {
        // Create a Glib::ustring copy of the buffer

        // Here we force the use of Glib::ustring::ustring( InputIterator begin, InputIterator end )
        // instead of Glib::ustring::ustring( const char*, size_type ) because it
        // expects the length of the string in characters, not in bytes.
        result = Glib::ustring( reinterpret_cast<const char *>(buffer), reinterpret_cast<const char *>(buffer + length) );

        // Deletes the original buffer
        xmlFree(buffer);
      }

      xmlFreeDoc(pDocOutput);
    }
  }

  return result;
}
