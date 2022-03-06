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

#include "opentx.h"
#include "widget.h"
#include "menu.h"
#include "widget_settings.h"
#include "view_main.h"
#include "lcd.h"

#if defined(HARDWARE_TOUCH)
#include "touch.h"
#endif

constexpr uint32_t WIDGET_FOCUS_TIMEOUT = 10*1000; // 10 seconds

static void openWidgetMenu(Widget * parent)
{
  Menu *menu = new Menu(parent);
  menu->addLine("Full screen", [=]() {
      parent->setFullscreen(true);
    });
  if(parent->getOptions() && parent->getOptions()->name)  
    menu->addLine(TR_WIDGET_SETTINGS, [=]() {
      new WidgetSettings(parent, parent);
    });
}

Widget::Widget(const WidgetFactory *factory, FormGroup *parent,
               const rect_t &rect, WidgetPersistentData *persistentData) :
    Button(parent, rect), factory(factory), persistentData(persistentData)
{
  setFocusHandler([&](bool focus) {
      if (focus) { // gained focus
        focusGainedTS = RTOS_GET_MS();
      }
    });

  setPressHandler([&]() -> uint8_t {
      openWidgetMenu(this);
      return 0;
    });
}

void Widget::checkEvents()
{
  Button::checkEvents();

  // Give the focus back to ViewMain after WIDGET_FOCUS_TIMEOUT milliseconds
  if (!fullscreen && hasFocus() && (RTOS_GET_MS() - focusGainedTS >= WIDGET_FOCUS_TIMEOUT)) {
    ViewMain::instance()->setFocus();
  }
}

#if defined(HARDWARE_TOUCH)
bool Widget::onTouchEnd(coord_t x, coord_t y)
{
  TRACE_WINDOWS("Widget received touch end (%d) x=%d;y=%d",
                hasFocus(), x, y);

  if (fullscreen) {
    //TODO: forward to widget (lua for instance)
    return true;
  }

  if (Window::onTouchEnd(x, y)) {
    return true;
  }
  
  if (hasFocus()) {
    if (touchState.tapCount == 0)
      onPress();
    else if (touchState.tapCount > 1)
      setFullscreen(true);
  }
  else {
    setFocus();
  }
  return true;
}

bool Widget::onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY,
                          coord_t slideX, coord_t slideY)
{
  TRACE_WINDOWS("Widget touch slide");
  if (fullscreen) { return true; }
  return false;
}

#endif

void Widget::paint(BitmapBuffer * dc)
{
  if (fullscreen) {
    // Draw background screen bellow
    OpenTxTheme::instance()->drawBackground(dc);
  }

  // refresh the widget
  refresh(dc);
  
  if (hasFocus() && !fullscreen) {

    // Blink from haft-time before expiring (5s)
    if ((RTOS_GET_MS() - focusGainedTS >= WIDGET_FOCUS_TIMEOUT / 2)
        && !FAST_BLINK_ON_PHASE) {
      return;
    }
    dc->drawRect(0, 0, width(), height(), 2, STASHED, COLOR_THEME_FOCUS);
  }
}

#if defined(HARDWARE_KEYS)
void Widget::onEvent(event_t event)
{
  TRACE("### event = 0x%x ###", event);
  if (!fullscreen) {
    if (event == EVT_KEY_BREAK(KEY_EXIT)) {
      // [EXIT] -> exit focus mode (if not fullscreen)
      killEvents(event);
      ViewMain::instance()->setFocus();
      return;
    }
    // Forward the rest to the parent class
    Button::onEvent(event);
  }
  // In fullscreen mode, we react only to that one key:
  // [RTN / EXIT LONG] -> exit fullscreen mode
  else if (EVT_KEY_LONG(KEY_EXIT) == event) {
    killEvents(event);
    setFullscreen(false);
  }
}
#endif

void Widget::update()
{
  auto container = dynamic_cast<WidgetsContainer*>(parent);
  if (container) {
    container->updateZones();
  }
}

void Widget::setFullscreen(bool fullscreen)
{
  if (fullscreen == this->fullscreen) return;

  // Leave Fullscreen Mode
  if (!fullscreen) {

    // Reset all zones in container
    Widget::update();
    setWindowFlags(getWindowFlags() & ~OPAQUE);

    // and give up focus
    ViewMain::instance()->setFocus();
    this->fullscreen = false;
  }
  // Enter Fullscreen Mode
  else {

    // Set window opaque (inhibits redraw from windows bellow)
    setWindowFlags(getWindowFlags() | OPAQUE);
    setRect(parent->getRect());
    setLeft(parent->getScrollPositionX());
    this->fullscreen = true;
    bringToTop();
  }
}

std::list<const WidgetFactory *> & getRegisteredWidgets()
{
  static std::list<const WidgetFactory *> widgets;
  return widgets;
}

void registerWidget(const WidgetFactory * factory)
{
  TRACE("register widget %s", factory->getName());
  getRegisteredWidgets().push_back(factory);
}

void unregisterWidget(const WidgetFactory * factory)
{
  TRACE("unregister widget %s", factory->getName());
  getRegisteredWidgets().remove(factory);
}

const WidgetFactory * getWidgetFactory(const char * name)
{
  auto it = getRegisteredWidgets().cbegin();
  for (; it != getRegisteredWidgets().cend();++it) {
    if (!strcmp(name, (*it)->getName())) {
      return (*it);
    }
  }
  return nullptr;
}

Widget * loadWidget(const char * name, FormGroup * parent, const rect_t & rect, WidgetPersistentData * persistentData)
{
  const WidgetFactory * factory = getWidgetFactory(name);
  if (factory) {
    return factory->create(parent, rect, persistentData, false);
  }
  return nullptr;
}
