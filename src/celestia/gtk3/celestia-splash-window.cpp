//
// Created by Chris Daley on 2/9/19.
//

#include <iostream>
#include "celestia-splash-window.h"

CelestiaSplashWindow::CelestiaSplashWindow()
    : Gtk::Window()
{ }

CelestiaSplashWindow::CelestiaSplashWindow(_GtkWindow *& win, Glib::RefPtr<Gtk::Builder> &)
    : Gtk::Window(win)
{ }

void CelestiaSplashWindow::screen_changed(const Glib::RefPtr<Gdk::Screen> &scr)
{
    auto visual = scr->get_rgba_visual();

    if (visual && scr->is_composited())
        gtk_widget_set_visual(GTK_WIDGET(this->gobj()), GDK_VISUAL(visual->gobj()));

}


Glib::RefPtr<CelestiaSplashWindow> CelestiaSplashWindow::create() {

    auto builder = Gtk::Builder::create();
    try
    {
        builder->add_from_resource("/builder/splash.ui");
    }
    catch (const Glib::Error& error)
    {
        std::cout << "Error loading example_builder.ui: " << error.what() << std::endl;
        return Glib::RefPtr<CelestiaSplashWindow>();
    }

    CelestiaSplashWindow* pWindow = nullptr;
    builder->get_widget_derived("Splash", pWindow);
    if (!pWindow)
    {
        std::cout << "Could not get 'Splash' from the builder." << std::endl;
        return Glib::RefPtr<CelestiaSplashWindow>();
    }

    pWindow->screen_changed(pWindow->get_screen());
    return Glib::RefPtr<CelestiaSplashWindow>(pWindow);

}