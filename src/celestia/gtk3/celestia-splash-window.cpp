//
// Created by Chris Daley on 2/9/19.
//

#include <iostream>
#include <gtkmm/main.h>
#include <gtkmm/fixed.h>
#include <gtkmm/image.h>
#include "celestia-splash-window.h"

CelestiaSplashWindow::CelestiaSplashWindow()
    : Gtk::Window()
{ }

CelestiaSplashWindow::CelestiaSplashWindow(_GtkWindow *& win, Glib::RefPtr<Gtk::Builder> & builder)
    : Gtk::Window(win)
{
    Gtk::Label* pLabel = nullptr;
    builder->get_widget("label", pLabel);
    Gtk::Image* pImage = nullptr;
    builder->get_widget("image", pImage);
    Gtk::Fixed* pFixed = nullptr;
    builder->get_widget("Fixed", pFixed);

    mImage = Glib::RefPtr<Gtk::Image>(pImage);
    mLabel = Glib::RefPtr<Gtk::Label>(pLabel);
    mFixed = Glib::RefPtr<Gtk::Fixed>(pFixed);
    mLabel->set_alignment(1, 1);
}

bool CelestiaSplashWindow::on_configure_event(GdkEventConfigure *configure_event){
    auto w = mImage->get_allocated_width();
    auto h = mImage->get_allocated_height();

    mLabel->set_size_request(mImage->get_allocated_width() - 80, mImage->get_allocated_height() / 2);
    mFixed->move(*mLabel.get(), 40, mImage->get_allocated_height() / 2 - 40);

    return false;
}

void CelestiaSplashWindow::screen_changed(const Glib::RefPtr<Gdk::Screen> &scr)
{
    auto visual = scr->get_rgba_visual();

    if (visual && scr->is_composited())
        gtk_widget_set_visual(GTK_WIDGET(this->gobj()), GDK_VISUAL(visual->gobj()));

}


void CelestiaSplashWindow::update(const std::string &filename)
{
    setText(filename.c_str());
}

void CelestiaSplashWindow::setText(const char *text)
{
    auto message = std::string("Version " VERSION "\n");
    mLabel->set_text(message + text);

    while (Gtk::Main::events_pending())
    {
        Gtk::Main::iteration();
    }
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