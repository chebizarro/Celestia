//
// Created by bizarro on 2/6/19.
//

#include "celestia-application.h"
#include "celestia-appwindow.h"

CelestiaApplication::CelestiaApplication()
        : Gtk::Application("org.celestia.application", Gio::APPLICATION_HANDLES_OPEN)
{
}

Glib::RefPtr<CelestiaApplication> CelestiaApplication::create()
{
    return Glib::RefPtr<CelestiaApplication>(new CelestiaApplication());
}

CelestiaAppWindow* CelestiaApplication::create_appwindow()
{
    auto appwindow = do_builder();

    // Make sure that the application runs for as long this window is still open.
    add_window(*appwindow);

    // Gtk::Application::add_window() connects a signal handler to the window's
    // signal_hide(). That handler removes the window from the application.
    // If it's the last window to be removed, the application stops running.
    // Gtk::Window::set_application() does not connect a signal handler, but is
    // otherwise equivalent to Gtk::Application::add_window().

    // Delete the window when it is hidden.
    appwindow->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this,
                                                                            &CelestiaApplication::on_hide_window), appwindow));

    return dynamic_cast<CelestiaAppWindow*>(appwindow);
}

void CelestiaApplication::on_activate()
{
    // The application has been started, so let's show a window.
    auto appwindow = create_appwindow();
    appwindow->present();
}

void CelestiaApplication::on_open(const Gio::Application::type_vec_files& files,
                                 const Glib::ustring& /* hint */)
{
    // The application has been asked to open some files,
    // so let's open a new view for each one.
    CelestiaAppWindow* appwindow = nullptr;
    auto windows = get_windows();
    if (windows.size() > 0)
        appwindow = dynamic_cast<CelestiaAppWindow*>(windows[0]);

    if (!appwindow)
        appwindow = create_appwindow();


    appwindow->present();
}

void CelestiaApplication::on_hide_window(Gtk::Window* window)
{
    //delete window;
}