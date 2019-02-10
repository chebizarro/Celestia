//
// Created by bizarro on 2/9/19.
//

#ifndef CELESTIA_CELESTIASPLASHWINDOW_H
#define CELESTIA_CELESTIASPLASHWINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>

class CelestiaSplashWindow : public Gtk::Window {

public:
    CelestiaSplashWindow();
    CelestiaSplashWindow(_GtkWindow*&, Glib::RefPtr<Gtk::Builder>&);
    static Glib::RefPtr<CelestiaSplashWindow> create();

protected:
    void screen_changed(const Glib::RefPtr<Gdk::Screen>& scr);

};


#endif //CELESTIA_CELESTIASPLASHWINDOW_H
