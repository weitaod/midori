/*
 Copyright (C) 2008 Christian Dywan <christian@twotoasts.de>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 See the file COPYING for the full license text.
*/

#include "midori-locationaction.h"

#include "main.h"
#include "gtkiconentry.h"

#include <katze/katze.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

struct _MidoriLocationAction
{
    GtkAction parent_instance;

    gchar* uri;
};

struct _MidoriLocationActionClass
{
    GtkActionClass parent_class;
};

G_DEFINE_TYPE (MidoriLocationAction, midori_location_action, GTK_TYPE_ACTION)

enum
{
    PROP_0,

    PROP_SECONDARY_ICON
};

enum
{
    ACTIVE_CHANGED,
    FOCUS_OUT,
    SECONDARY_ICON_RELEASED,
    RESET_URI,
    SUBMIT_URI,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static void
midori_location_action_finalize (GObject* object);

static void
midori_location_action_set_property (GObject*      object,
                                     guint         prop_id,
                                     const GValue* value,
                                     GParamSpec*   pspec);

static void
midori_location_action_activate (GtkAction* object);

static GtkWidget*
midori_location_action_create_tool_item (GtkAction* action);

static void
midori_location_action_connect_proxy (GtkAction* action,
                                      GtkWidget* proxy);

static void
midori_location_action_disconnect_proxy (GtkAction* action,
                                         GtkWidget* proxy);

static void
midori_location_action_class_init (MidoriLocationActionClass* class)
{
    GObjectClass* gobject_class;
    GtkActionClass* action_class;

    signals[ACTIVE_CHANGED] = g_signal_new ("active-changed",
                                            G_TYPE_FROM_CLASS (class),
                                            (GSignalFlags) (G_SIGNAL_RUN_LAST),
                                            0,
                                            0,
                                            NULL,
                                            g_cclosure_marshal_VOID__INT,
                                            G_TYPE_NONE, 1,
                                            G_TYPE_INT);

    signals[FOCUS_OUT] = g_signal_new ("focus-out",
                                       G_TYPE_FROM_CLASS (class),
                                       (GSignalFlags) (G_SIGNAL_RUN_LAST),
                                       0,
                                       0,
                                       NULL,
                                       g_cclosure_marshal_VOID__VOID,
                                       G_TYPE_NONE, 0);

    signals[SECONDARY_ICON_RELEASED] = g_signal_new ("secondary-icon-released",
                                       G_TYPE_FROM_CLASS (class),
                                       (GSignalFlags) (G_SIGNAL_RUN_LAST),
                                       0,
                                       0,
                                       NULL,
                                       g_cclosure_marshal_VOID__OBJECT,
                                       G_TYPE_NONE, 1,
                                       GTK_TYPE_WIDGET);

    signals[RESET_URI] = g_signal_new ("reset-uri",
                                       G_TYPE_FROM_CLASS (class),
                                       (GSignalFlags) (G_SIGNAL_RUN_LAST),
                                       0,
                                       0,
                                       NULL,
                                       g_cclosure_marshal_VOID__VOID,
                                       G_TYPE_NONE, 0);

    signals[SUBMIT_URI] = g_signal_new ("submit-uri",
                                        G_TYPE_FROM_CLASS (class),
                                        (GSignalFlags) (G_SIGNAL_RUN_LAST),
                                        0,
                                        0,
                                        NULL,
                                        g_cclosure_marshal_VOID__STRING,
                                        G_TYPE_NONE, 1,
                                        G_TYPE_STRING);

    gobject_class = G_OBJECT_CLASS (class);
    gobject_class->finalize = midori_location_action_finalize;
    gobject_class->set_property = midori_location_action_set_property;

    action_class = GTK_ACTION_CLASS (class);
    action_class->activate = midori_location_action_activate;
    action_class->create_tool_item = midori_location_action_create_tool_item;
    action_class->connect_proxy = midori_location_action_connect_proxy;
    action_class->disconnect_proxy = midori_location_action_disconnect_proxy;

    g_object_class_install_property (gobject_class,
                                     PROP_SECONDARY_ICON,
                                     g_param_spec_string (
                                     "secondary-icon",
                                     "Secondary",
                                     _("The stock ID of the secondary icon"),
                                     NULL,
                                     G_PARAM_WRITABLE));
}

static void
midori_location_action_init (MidoriLocationAction* location_action)
{
    location_action->uri = NULL;
}

static void
midori_location_action_finalize (GObject* object)
{
    MidoriLocationAction* location_action = MIDORI_LOCATION_ACTION (object);

    g_free (location_action->uri);

    G_OBJECT_CLASS (midori_location_action_parent_class)->finalize (object);
}

static void
midori_location_action_set_property (GObject*      object,
                                     guint         prop_id,
                                     const GValue* value,
                                     GParamSpec*   pspec)
{
    MidoriLocationAction* location_action = MIDORI_LOCATION_ACTION (object);

    switch (prop_id)
    {
    case PROP_SECONDARY_ICON:
    {
        midori_location_action_set_secondary_icon (location_action,
            g_value_get_string (value));
        break;
    }
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
midori_location_action_activate (GtkAction* action)
{
    GSList* proxies;
    GtkWidget* alignment;
    GtkWidget* entry;

    proxies = gtk_action_get_proxies (action);
    if (!proxies)
        return;

    do
    if (GTK_IS_TOOL_ITEM (proxies->data))
    {
        alignment = gtk_bin_get_child (GTK_BIN (proxies->data));
        entry = gtk_bin_get_child (GTK_BIN (alignment));

        /* Obviously only one widget can end up with the focus.
        Yet we can't predict which one that is, can we? */
        gtk_widget_grab_focus (entry);
    }
    while ((proxies = g_slist_next (proxies)));

    if (GTK_ACTION_CLASS (midori_location_action_parent_class)->activate)
        GTK_ACTION_CLASS (midori_location_action_parent_class)->activate (action);
}

static GtkWidget*
midori_location_action_create_tool_item (GtkAction* action)
{
    GtkWidget* toolitem;
    GtkWidget* entry;
    GtkWidget* alignment;

    toolitem = GTK_WIDGET (gtk_tool_item_new ());
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (toolitem), TRUE);
    entry = midori_location_entry_new ();
    gtk_icon_entry_set_icon_highlight (GTK_ICON_ENTRY (
        gtk_bin_get_child (GTK_BIN (entry))),
        GTK_ICON_ENTRY_SECONDARY, TRUE);
    alignment = gtk_alignment_new (0, 0.5, 1, 0.1);
    gtk_container_add (GTK_CONTAINER (alignment), entry);
    gtk_widget_show (entry);
    gtk_container_add (GTK_CONTAINER (toolitem), alignment);
    gtk_widget_show (alignment);

    return GTK_WIDGET (toolitem);
}

static void
midori_location_action_active_changed_cb (GtkWidget* widget,
                                          gint       active,
                                          GtkAction* action)
{
    MidoriLocationAction* location_action;
    const gchar* text;

    location_action = MIDORI_LOCATION_ACTION (action);
    text = midori_location_entry_get_text (MIDORI_LOCATION_ENTRY (widget));
    katze_assign (location_action->uri, g_strdup (text));

    g_signal_emit (action, signals[ACTIVE_CHANGED], 0, active);
}

static gboolean
midori_location_action_key_press_event_cb (GtkWidget*   widget,
                                           GdkEventKey* event,
                                           GtkAction*   action)
{
    const gchar* uri;

    switch (event->keyval)
    {
    case GDK_ISO_Enter:
    case GDK_KP_Enter:
    case GDK_Return:
    {
        if ((uri = gtk_entry_get_text (GTK_ENTRY (widget))))
        {
            g_signal_emit (action, signals[SUBMIT_URI], 0, uri);
            return TRUE;
        }
    }
    case GDK_Escape:
    {
        g_signal_emit (action, signals[RESET_URI], 0);
        return TRUE;
    }
    }
    return FALSE;
}

static gboolean
midori_location_action_focus_out_event_cb (GtkWidget*   widget,
                                           GdkEventKey* event,
                                           GtkAction*   action)
{
    g_signal_emit (action, signals[FOCUS_OUT], 0);
    return FALSE;
}

static void
midori_location_action_icon_released_cb (GtkWidget*           widget,
                                         GtkIconEntryPosition icon_pos,
                                         gint                 button,
                                         GtkAction*           action)
{
    if (icon_pos == GTK_ICON_ENTRY_SECONDARY)
        g_signal_emit (action, signals[SECONDARY_ICON_RELEASED], 0, widget);
}

static void
midori_location_action_connect_proxy (GtkAction* action,
                                      GtkWidget* proxy)
{
    GtkWidget* alignment;
    GtkWidget* entry;

    GTK_ACTION_CLASS (midori_location_action_parent_class)->connect_proxy (
        action, proxy);

    if (GTK_IS_TOOL_ITEM (proxy))
    {
        alignment = gtk_bin_get_child (GTK_BIN (proxy));
        entry = gtk_bin_get_child (GTK_BIN (alignment));

        g_signal_connect (entry, "active-changed",
            G_CALLBACK (midori_location_action_active_changed_cb), action);
        g_object_connect (gtk_bin_get_child (GTK_BIN (entry)),
                      "signal::key-press-event",
                      midori_location_action_key_press_event_cb, action,
                      "signal::focus-out-event",
                      midori_location_action_focus_out_event_cb, action,
                      "signal::icon-released",
                      midori_location_action_icon_released_cb, action,
                      NULL);
    }
}

static void
midori_location_action_disconnect_proxy (GtkAction* action,
                                         GtkWidget* proxy)
{
    g_signal_handlers_disconnect_by_func (proxy,
        G_CALLBACK (gtk_action_activate), action);

    GTK_ACTION_CLASS (midori_location_action_parent_class)->disconnect_proxy
        (action, proxy);
}

const gchar*
midori_location_action_get_uri (MidoriLocationAction* location_action)
{
    g_return_val_if_fail (MIDORI_IS_LOCATION_ACTION (location_action), NULL);

    return location_action->uri;
}

void
midori_location_action_set_uri (MidoriLocationAction* location_action,
                                const gchar*          uri)
{
    GSList* proxies;
    GtkWidget* alignment;
    GtkWidget* entry;

    g_return_if_fail (MIDORI_IS_LOCATION_ACTION (location_action));
    g_return_if_fail (uri != NULL);


    katze_assign (location_action->uri, g_strdup (uri));

    proxies = gtk_action_get_proxies (GTK_ACTION (location_action));
    if (!proxies)
        return;

    do
    if (GTK_IS_TOOL_ITEM (proxies->data))
    {
        alignment = gtk_bin_get_child (GTK_BIN (proxies->data));
        entry = gtk_bin_get_child (GTK_BIN (alignment));

        midori_location_entry_set_text (MIDORI_LOCATION_ENTRY (entry), uri);
    }
    while ((proxies = g_slist_next (proxies)));
}

void
midori_location_action_add_uri (MidoriLocationAction* location_action,
                                const gchar*          uri)
{
    GSList* proxies;
    GtkWidget* alignment;
    GtkWidget* entry;
    MidoriLocationEntryItem item;

    g_return_if_fail (MIDORI_IS_LOCATION_ACTION (location_action));
    g_return_if_fail (uri != NULL);


    katze_assign (location_action->uri, g_strdup (uri));

    proxies = gtk_action_get_proxies (GTK_ACTION (location_action));
    if (!proxies)
        return;

    do
    if (GTK_IS_TOOL_ITEM (proxies->data))
    {
        alignment = gtk_bin_get_child (GTK_BIN (proxies->data));
        entry = gtk_bin_get_child (GTK_BIN (alignment));

        item.favicon = NULL;
        item.uri = uri;
        item.title = NULL;
        midori_location_entry_add_item (
            MIDORI_LOCATION_ENTRY (entry), &item);
    }
    while ((proxies = g_slist_next (proxies)));
}

void
midori_location_action_set_icon_for_uri (MidoriLocationAction* location_action,
                                         GdkPixbuf*            icon,
                                         const gchar*          uri)
{
    GSList* proxies;
    GtkWidget* alignment;
    GtkWidget* entry;
    GtkWidget* child;
    MidoriLocationEntryItem item;

    g_return_if_fail (MIDORI_IS_LOCATION_ACTION (location_action));
    g_return_if_fail (!icon || GDK_IS_PIXBUF (icon));
    g_return_if_fail (uri != NULL);

    proxies = gtk_action_get_proxies (GTK_ACTION (location_action));
    if (!proxies)
        return;

    do
    if (GTK_IS_TOOL_ITEM (proxies->data))
    {
        alignment = gtk_bin_get_child (GTK_BIN (proxies->data));
        entry = gtk_bin_get_child (GTK_BIN (alignment));
        child = gtk_bin_get_child (GTK_BIN (entry));

        item.favicon = icon;
        item.uri = uri;
        item.title = NULL;
        midori_location_entry_add_item (
            MIDORI_LOCATION_ENTRY (entry), &item);
        if (location_action->uri == uri)
            gtk_icon_entry_set_icon_from_pixbuf (GTK_ICON_ENTRY (child),
                GTK_ICON_ENTRY_PRIMARY, icon);
    }
    while ((proxies = g_slist_next (proxies)));
}

void
midori_location_action_set_title_for_uri (MidoriLocationAction* location_action,
                                          const gchar*          title,
                                          const gchar*          uri)
{
    GSList* proxies;
    GtkWidget* alignment;
    GtkWidget* entry;
    MidoriLocationEntryItem item;

    g_return_if_fail (MIDORI_IS_LOCATION_ACTION (location_action));
    g_return_if_fail (title != NULL);
    g_return_if_fail (uri != NULL);

    proxies = gtk_action_get_proxies (GTK_ACTION (location_action));
    if (!proxies)
        return;

    do
    if (GTK_IS_TOOL_ITEM (proxies->data))
    {
        alignment = gtk_bin_get_child (GTK_BIN (proxies->data));
        entry = gtk_bin_get_child (GTK_BIN (alignment));

        item.favicon = NULL;
        item.uri = uri;
        item.title = item.uri;
        midori_location_entry_add_item (
            MIDORI_LOCATION_ENTRY (entry), &item);
    }
    while ((proxies = g_slist_next (proxies)));
}

void
midori_location_action_set_secondary_icon (MidoriLocationAction* location_action,
                                           const gchar*          stock_id)
{
    GSList* proxies;
    GtkWidget* alignment;
    GtkWidget* entry;
    GtkWidget* child;
    GtkStockItem stock_item;

    g_return_if_fail (MIDORI_IS_LOCATION_ACTION (location_action));
    g_return_if_fail (!stock_id || gtk_stock_lookup (stock_id, &stock_item));

    proxies = gtk_action_get_proxies (GTK_ACTION (location_action));
    if (!proxies)
        return;

    do
    if (GTK_IS_TOOL_ITEM (proxies->data))
    {
        alignment = gtk_bin_get_child (GTK_BIN (proxies->data));
        entry = gtk_bin_get_child (GTK_BIN (alignment));
        child = gtk_bin_get_child (GTK_BIN (entry));

        if (stock_id)
            gtk_icon_entry_set_icon_from_stock (GTK_ICON_ENTRY (child),
                GTK_ICON_ENTRY_SECONDARY, stock_id);
        else
            gtk_icon_entry_set_icon_from_pixbuf (GTK_ICON_ENTRY (child),
                GTK_ICON_ENTRY_SECONDARY, NULL);
    }
    while ((proxies = g_slist_next (proxies)));
}
