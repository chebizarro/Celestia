//
// Created by bizarro on 2/6/19.
//

#ifndef CELESTIA_CELESTIA_APPWINDOW_H

#include <gtkmm.h>
#include <celestia/celestiacore.h>
#include <memory>

class CelestiaAppWindow : public Gtk::ApplicationWindow
{
public:
    CelestiaAppWindow();
    CelestiaAppWindow(_GtkApplicationWindow*&, Glib::RefPtr<Gtk::Builder>&);

protected:
    void realize();
    void unrealize();
    bool render(const Glib::RefPtr<Gdk::GLContext>& /* context */);

private:
    Glib::RefPtr<Gtk::GLArea> mGLArea;
    std::unique_ptr<CelestiaCore> mCore;
    Renderer* mRenderer;
    Simulation* mSimulation;
};

Gtk::Window* do_builder();


#define CELESTIA_CELESTIA_APPWINDOW_H

#endif //CELESTIA_CELESTIA_APPWINDOW_H
