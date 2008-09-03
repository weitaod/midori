/*
 Copyright (C) 2007-2008 Christian Dywan <christian@twotoasts.de>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 See the file COPYING for the full license text.
*/

#ifndef __MAIN_H__
#define __MAIN_H__ 1

#include <gtk/gtk.h>

/* Custom stock items

   We should distribute these
   Names should match with epiphany and/ or xdg spec */

#define STOCK_BOOKMARK           "stock_bookmark"
#define STOCK_BOOKMARKS          "vcard"
#define STOCK_CONSOLE            "terminal"
#define STOCK_DISABLE            "list-remove"
#define STOCK_ENABLE             "list-add"
#define STOCK_EXTENSION          "extension"
#define STOCK_EXTENSIONS         "extension"
#define STOCK_HISTORY            "document-open-recent"
#define STOCK_NEWS_FEED          "news-feed"
#define STOCK_PAGE_HOLDER        "page-holder"
#define STOCK_STYLE              "gnome-settings-theme"
#define STOCK_STYLES             "gnome-settings-theme"
#define STOCK_TRANSFER           "package"
#define STOCK_TRANSFERS          "package"

/* We assume that these legacy icon names are usually present */

#define STOCK_BOOKMARK_ADD       "stock_add-bookmark"
#define STOCK_HOMEPAGE           GTK_STOCK_HOME
#define STOCK_IMAGE              "gnome-mime-image"
#define STOCK_NETWORK_OFFLINE    "network-offline"
#define STOCK_SCRIPT             "stock_script"
#define STOCK_SCRIPTS            "stock_script"
#define STOCK_SEND               "stock_mail-send"
#define STOCK_TAB_NEW            "stock_new-tab"
#define STOCK_USER_TRASH         "gnome-stock-trash"
#define STOCK_WINDOW_NEW         "stock_new-window"

/* For backwards compatibility */

#if !GTK_CHECK_VERSION(2, 10, 0)
#define GTK_STOCK_SELECT_ALL     "gtk-select-all"
#endif
#if !GTK_CHECK_VERSION(2, 8, 0)
#define GTK_STOCK_FULLSCREEN "gtk-fullscreen"
#define GTK_STOCK_LEAVE_FULLSCREEN "gtk-leave-fullscreen"
#endif

#endif /* !__MAIN_H__ */
