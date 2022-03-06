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

#ifndef _MESSAGE_DIALOG_H_
#define _MESSAGE_DIALOG_H_

#include "dialog.h"
#include "static.h"

class MessageDialog: public Dialog {
  public:
    MessageDialog(Window * parent, const char * title, const char * message, const char * info = "");

    void setInfoText(std::string text)
    {
      infoWidget->setText(std::move(text));
    }

  protected:
    StaticText * messageWidget;
    StaticText * infoWidget;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MessageDialog";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif
};

class DynamicMessageDialog : public Dialog
{
 public:
  DynamicMessageDialog(Window* parent, const char* title, 
                std::function<std::string()> textHandler, const char* message = "", const int lineHeight = PAGE_LINE_HEIGHT,
                const LcdFlags textFlags = CENTERED);
  // Attn.: FONT(XXL) is not supported by DynamicMessageDialog


 protected:
  StaticText* messageWidget;
  DynamicText* infoWidget;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "DynamicMessageDialog"; }
#endif

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif
};


#endif // _MESSAGE_DIALOG_H_
