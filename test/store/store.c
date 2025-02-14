/**
 * @file
 * Test code for the store
 *
 * @authors
 * Copyright (C) 2020 Richard Russon <rich@flatcap.org>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define TEST_NO_MAIN
#include "config.h"
#include "acutest.h"
#include <stddef.h>
#include <stdbool.h>
#include "mutt/lib.h"
#include "store/lib.h"

void test_store_store(void)
{
  const char *list = NULL;

  list = store_backend_list();
  TEST_CHECK(list != NULL);
  FREE(&list);

  const struct StoreOps *sops = NULL;

  sops = store_get_backend_ops(NULL);
  TEST_CHECK(sops != NULL);

#ifdef HAVE_TC
  sops = store_get_backend_ops("tokyocabinet");
  TEST_CHECK(sops != NULL);
#endif

  TEST_CHECK(store_is_valid_backend(NULL) == true);

#ifdef HAVE_TC
  TEST_CHECK(store_is_valid_backend("tokyocabinet") == true);
#endif
}
