/* SPDX-FileCopyrightText: 2008 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup edinterface
 *
 * General Interface Region Code
 *
 * \note Most logic is now in 'interface_region_*.c'
 */

#include "BLI_listbase.h"

#include "BKE_context.hh"
#include "BKE_screen.hh"

#include "wm_draw.hh"

#include "ED_screen.hh"

#include "interface_regions_intern.hh"

ARegion *ui_region_temp_add(bScreen *screen)
{
  ARegion *region = BKE_area_region_new();
  BLI_addtail(&screen->regionbase, region);

  region->regiontype = RGN_TYPE_TEMPORARY;
  region->alignment = RGN_ALIGN_FLOAT;

  return region;
}

void ui_region_temp_remove(bContext *C, bScreen *screen, ARegion *region)
{
  wmWindow *win = CTX_wm_window(C);

  BLI_assert(region->regiontype == RGN_TYPE_TEMPORARY);
  BLI_assert(BLI_findindex(&screen->regionbase, region) != -1);
  if (win) {
    wm_draw_region_clear(win, region);
  }

  ED_region_exit(C, region);
  BKE_area_region_free(nullptr, region); /* nullptr: no space-type. */
  BLI_freelinkN(&screen->regionbase, region);

  if (CTX_wm_region(C) == region) {
    CTX_wm_region_set(C, nullptr);
  }
  if (CTX_wm_region_popup(C) == region) {
    CTX_wm_region_popup_set(C, nullptr);
  }
}
