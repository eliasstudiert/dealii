// ---------------------------------------------------------------------
//
// Copyright (C) 2010 - 2018 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------


// make sure we print something at all when an entry in the table corresponds
// to the empty string


#include <deal.II/base/data_out_base.h>
#include <deal.II/base/table_handler.h>

#include <string>
#include <vector>

#include "../tests.h"


int
main()
{
  initlog();

  TableHandler table;

  std::string keys[3] = {"key1", "key2", "key3"};

  table.add_value(keys[0], 0);
  table.add_value(keys[1], std::string(""));
  table.add_value(keys[2], std::string("a"));

  table.write_text(deallog.get_file_stream());
}
