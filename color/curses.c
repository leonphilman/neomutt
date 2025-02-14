/**
 * @file
 * Curses Colour
 *
 * @authors
 * Copyright (C) 2021 Richard Russon <rich@flatcap.org>
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

/**
 * @page color_curses Curses Colour
 *
 * A wrapper that represents a colour in Curses.
 */

#include "config.h"
#include <stddef.h>
#include <stdbool.h>
#include "mutt/lib.h"
#include "gui/lib.h"
#include "lib.h"

struct CursesColorList CursesColors; ///< List of all Curses colours
int NumCursesColors;

/**
 * curses_colors_init - Initialise the Curses colours
 */
void curses_colors_init(void)
{
  color_debug(LL_DEBUG5, "init CursesColors\n");
  TAILQ_INIT(&CursesColors);
  NumCursesColors = 0;
}

/**
 * curses_colors_find - Find a Curses colour by foreground/background
 * @param fg Foreground colour
 * @param bg Background colour
 * @retval ptr Curses colour
 */
struct CursesColor *curses_colors_find(int fg, int bg)
{
  struct CursesColor *cc = NULL;
  TAILQ_FOREACH(cc, &CursesColors, entries)
  {
    if ((cc->fg == fg) && (cc->bg == bg))
    {
      return cc;
    }
  }

  return NULL;
}

/**
 * curses_color_init - Initialise a new Curses colour
 * @param fg Foreground colour
 * @param bg Background colour
 * @retval num Index of Curses colour
 */
static int curses_color_init(int fg, int bg)
{
  color_debug(LL_DEBUG5, "find lowest index\n");
  int index = 16;
  struct CursesColor *cc = NULL;
  TAILQ_FOREACH(cc, &CursesColors, entries)
  {
    if (cc->index == index)
      index++;
    else
      break;
  }
  color_debug(LL_DEBUG5, "lowest index = %d\n", index);
  if (index >= COLOR_PAIRS)
  {
    static bool warned = false;
    if (!warned)
    {
      mutt_error(_("Too many colors: %d / %d"), index, COLOR_PAIRS);
      warned = true;
    }
    return 0;
  }

  // const char *color = color_debug_log_color(fg, bg);
  // printf("%s\n", color);
  if (fg == COLOR_DEFAULT)
    fg = COLOR_UNSET;
  if (bg == COLOR_DEFAULT)
    bg = COLOR_UNSET;

  int rc = init_pair(index, fg, bg);
  color_debug(LL_DEBUG5, "init_pair(%d,%d,%d) -> %d\n", index, fg, bg, rc);

  return index;
}

/**
 * curses_color_free - Free a CursesColor
 * @param ptr CursesColor to be freed
 */
void curses_color_free(struct CursesColor **ptr)
{
  if (!ptr || !*ptr)
    return;

  struct CursesColor *cc = *ptr;
  if (cc->ref_count > 1)
  {
    cc->ref_count--;
    curses_color_dump(cc, "CursesColor rc--: ");
    *ptr = NULL;
    // curses_colors_dump();
    return;
  }

  curses_color_dump(cc, "free: ");
  TAILQ_REMOVE(&CursesColors, cc, entries);
  NumCursesColors--;
  color_debug(LL_DEBUG5, "CursesColors: %d\n", NumCursesColors);
  // curses_colors_dump();
  FREE(ptr);
}

/**
 * curses_color_new - Create a new CursesColor
 * @param fg Foreground colour
 * @param bg Background colour
 * @retval ptr New CursesColor
 *
 * If the colour already exists, this function will return a pointer to the
 * object (and increase its ref-count).
 */
struct CursesColor *curses_color_new(int fg, int bg)
{
  color_debug(LL_DEBUG5, "fg %d, bg %d\n", fg, bg);
  if (((fg == COLOR_UNSET) && (bg == COLOR_UNSET)) ||
      ((fg == COLOR_DEFAULT) && (bg == COLOR_DEFAULT)))
  {
    color_debug(LL_DEBUG5, "both unset\n");
    return NULL;
  }

  struct CursesColor *cc = curses_colors_find(fg, bg);
  if (cc)
  {
    cc->ref_count++;
    curses_color_dump(cc, "rc++: ");
    return cc;
  }

  color_debug(LL_DEBUG5, "new curses\n");
  int index = curses_color_init(fg, bg);
  if (index < 0)
    return NULL;

  struct CursesColor *cc_new = mutt_mem_calloc(1, sizeof(*cc_new));
  NumCursesColors++;
  color_debug(LL_DEBUG5, "CursesColor %p\n", cc_new);
  cc_new->fg = fg;
  cc_new->bg = bg;
  cc_new->ref_count = 1;
  cc_new->index = index;

  // insert curses colour
  TAILQ_FOREACH(cc, &CursesColors, entries)
  {
    if (cc->index > index)
    {
      color_debug(LL_DEBUG5, "insert\n");
      TAILQ_INSERT_BEFORE(cc, cc_new, entries);
      goto done;
    }
  }

  TAILQ_INSERT_TAIL(&CursesColors, cc_new, entries);
  color_debug(LL_DEBUG5, "tail\n");

done:
  curses_color_dump(cc_new, "CursesColor new: ");
  color_debug(LL_DEBUG5, "CursesColors: %d\n", NumCursesColors);
  return cc_new;
}
