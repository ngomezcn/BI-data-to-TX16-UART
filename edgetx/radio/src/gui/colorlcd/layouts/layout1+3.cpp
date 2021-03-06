/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "layout.h"
#include "layout_factory_impl.h"

const uint8_t LBM_LAYOUT_1P3[] = {
#include "mask_layout1+3.lbm"
};

const ZoneOption OPTIONS_LAYOUT_1P3[] =  {
  LAYOUT_COMMON_OPTIONS,
  LAYOUT_OPTIONS_END
};

class Layout1P3: public Layout
{
  public:
    Layout1P3(const LayoutFactory * factory, Layout::PersistentData * persistentData):
      Layout(factory, persistentData)
    {
    }

    unsigned int getZonesCount() const override
    {
      return 4;
    }

    rect_t getZone(unsigned int index) const override
    {
      rect_t zone = getMainZone();

      zone.w /= 2;

      if (index == 0) {
        if (isMirrored()) {
          zone.x += zone.w;
        }
      } else {
        index--;
        zone.h /= 3;
        zone.y += zone.h * (int)index;
        if (!isMirrored()) {
          zone.x += zone.w;
        }
      }

      return zone;
    }
};

BaseLayoutFactory<Layout1P3> Layout1P3("Layout1P3", "1 + 3", LBM_LAYOUT_1P3, OPTIONS_LAYOUT_1P3);
