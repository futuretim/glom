/* Glom
 *
 * Copyright (C) 2012 Openismus GmbH
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
71 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <libglom/document/document.h>
#include <libglom/translations_po.h>
#include <libglom/init.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>

#include <iostream>

 
int main()
{
  Glom::libglom_init();

  // Get a URI for a test file:
  Glib::ustring uri;

  try
  {
    const std::string path =
       Glib::build_filename(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED,
         "example_film_manager.glom");
    uri = Glib::filename_to_uri(path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return EXIT_FAILURE;
  }

  //std::cout << "URI=" << uri << std::endl;


  // Load the document:
  Glom::Document document;
  document.set_file_uri(uri);
  int failure_code = 0;
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << "Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  document.set_allow_autosave(false); //Do not save changes back to the example file:
  
  Glib::ustring po_file_uri;
  try
  {
    const std::string path =
      Glib::build_filename(GLOM_TESTS_TRANSLATIONS_PO_DATA_NOTINSTALLED,
        "test.po");
    po_file_uri = Glib::filename_to_uri(path);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what();
    return EXIT_FAILURE;
  }

  if(po_file_uri.empty())
  {
    std::cerr << "po_file_uri was empty." << std::endl;
    return EXIT_FAILURE;
  }

  //std::cout << "po file URI: " << po_file_uri << std::endl;

  const Glib::ustring locale = "de_DE";
  const bool success = 
    Glom::import_translations_from_po_file(&document, po_file_uri, locale);
  if(!success)
  {
    std::cerr << "Glom::import_translations_from_po_file() failed." << std::endl;
    return EXIT_FAILURE;
  }


  //Check that some expected translated titles are now in the document:
  Glom::sharedptr<const Glom::TableInfo> table = document.get_table("scenes");
  g_assert(table);
  g_assert( table->get_title() == "Scenes" ); //The original title should be unchanged:

  //This should have a new translated title:
  if(table->get_title_translation(locale) != "TestResult1")
  {
    std::cerr << "Failure: Unexpected translated report title." << std::endl;
    return EXIT_FAILURE;
  }

  const Glom::sharedptr<const Glom::Report> report = document.get_report("crew", "crew_list");
  g_assert(report);
  g_assert(report->get_title() == "Crew List"); //The original title should be unchanged:

  //This should have a new translated title:
  if(report->get_title_translation(locale) != "TestResult2")
  {
    std::cerr << "Failure: Unexpected translated report title." << std::endl;
    return EXIT_FAILURE;
  }

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
