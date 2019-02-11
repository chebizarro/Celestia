//
// Created by bizarro on 2/6/19.
//

#ifndef CELESTIA_CELESTIA_APPWINDOW_H

#include <gtkmm.h>
#include <celestia/celestiacore.h>
#include <memory>
#include "celestia-app-data.h"

class CelestiaAppWindow : public Gtk::ApplicationWindow
{
public:
    CelestiaAppWindow();
    CelestiaAppWindow(_GtkApplicationWindow*&, Glib::RefPtr<Gtk::Builder>&);
    static Glib::RefPtr<CelestiaAppWindow> create(std::shared_ptr<CelestiaAppData>);

protected:
    void gl_realize();
    void gl_unrealize();
    bool gl_render(const Glib::RefPtr<Gdk::GLContext>& /* context */);
    bool gl_configure(GdkEventConfigure *configure_event);
    bool gl_idle();

private:
    void set_app_data(std::shared_ptr<CelestiaAppData>);

    Glib::RefPtr<Gtk::GLArea> mGLArea;
    Glib::RefPtr<Gtk::Builder> mBuilder;
    Glib::RefPtr<Gio::Settings> mSettings;
    std::shared_ptr<CelestiaAppData> mAppData;

};


#define CELESTIA_CELESTIA_APPWINDOW_H

#endif //CELESTIA_CELESTIA_APPWINDOW_H
