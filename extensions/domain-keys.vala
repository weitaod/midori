/*
 Copyright (C) 2014 James Axl <bilimish@yandex.ru>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 See the file COPYING for the full license text.
*/

namespace DomainHotkeys {
    class Manager : Midori.Extension {
        internal Manager () {
            GLib.Object (name: _("Domain Hotkeys"),
                         description: _("Add www. and .com/.country_domain and proceed with Ctrl+Enter/Ctrl+Shift"),
                         version: "0.1" + Midori.VERSION_SUFFIX,
                         authors: "James Axl <bilimish@yandex.ru>");
            activate.connect (this.activated);
            deactivate.connect (this.deactivated);
        }

        bool key_press_event (Midori.LocationAction action, Gdk.EventKey event_key) {
            if (event_key.keyval == Gdk.Key.Return) {
                if ((bool)(event_key.state & Gdk.ModifierType.CONTROL_MASK)) {
                    submit_uri(action);
                    return true;
                } else if((bool)(event_key.state & Gdk.ModifierType.SHIFT_MASK)) {
                    submit_uri(action, true);
                    return true;
                }
            }
            return false;
        }
        
        void submit_uri(Midori.LocationAction action, bool locale = false) {
			var url = action.get_text ();
			if (locale){
				var domain = C_("Domain", ".com");
				url = "www." + url + domain;
			} else {
				url = "www." + url + ".com";
			}
			action.submit_uri(url, false);
		}

        void browser_added (Midori.Browser browser) {
            var action_group = browser.get_action_group ();
            var action = action_group.get_action ("Location") as Midori.LocationAction;
            action.key_press_event.connect (key_press_event);
        }

        void activated (Midori.App app) {
            foreach (var browser in app.get_browsers ())
                browser_added (browser);
            app.add_browser.connect (browser_added);
        }

        void browser_removed (Midori.Browser browser) {
            var action_group = browser.get_action_group ();
            var action = action_group.get_action ("Location") as Midori.LocationAction;
            action.key_press_event.disconnect (key_press_event);
        }

        void deactivated () {
            var app = get_app ();
            app.add_browser.disconnect (browser_added);
            foreach (var browser in app.get_browsers ())
                browser_removed (browser);
        }
    }
}

public Midori.Extension extension_init () {
    return new DomainHotkeys.Manager ();
}
