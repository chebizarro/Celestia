//
// Created by bizarro on 2/6/19.
//

#include "celestia-appwindow.h"
#include <gtk/gtk.h>
#include <iostream>
#include <GL/glew.h>

CelestiaAppWindow::CelestiaAppWindow()
        : Gtk::ApplicationWindow()
{
}


CelestiaAppWindow::CelestiaAppWindow(_GtkApplicationWindow*& win, Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(win)
{
    Gtk::GLArea* pGLArea = nullptr;
    builder->get_widget("GLArea", pGLArea);
    mGLArea = Glib::RefPtr<Gtk::GLArea>(pGLArea);

    mGLArea->set_hexpand(true);
    mGLArea->set_vexpand(true);

    mGLArea->signal_realize().connect(sigc::mem_fun(*this, &CelestiaAppWindow::realize));
    mGLArea->signal_unrealize().connect(sigc::mem_fun(*this, &CelestiaAppWindow::unrealize), false);
    mGLArea->signal_render().connect(sigc::mem_fun(*this, &CelestiaAppWindow::render), false);

    mCore = std::make_unique<CelestiaCore>();

    vector<std::string> configDirs;
    mCore->initSimulation("", configDirs, nullptr);
    mSimulation = mCore->getSimulation();
    mRenderer = mCore->getRenderer();
    mRenderer->setSolarSystemMaxDistance(mCore->getConfig()->SolarSystemMaxDistance);

}

void CelestiaAppWindow::realize()
{
    mGLArea->make_current();
    try
    {
        mGLArea->throw_if_error();

        GLenum glewErr = glewInit();
        {
            if (GLEW_OK != glewErr)
            {
                std::cerr << "Celestia was unable to initialize OpenGL extensions. Graphics quality will be reduced. Only Basic render path will be available." << std::endl;
            }
        }

        if (!mCore->initRenderer())
        {
            std::cerr << "Failed to initialize renderer.\n";
        }

        mCore->start((double)time(NULL) / 86400.0 + (double)astro::Date(1970, 1, 1));
        mCore->setTimeZoneName("UTC");

    }
    catch(const Gdk::GLError& gle)
    {
        std::cerr << "An error occured making the context current during realize:" << std::endl;
        std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
    }
}

void CelestiaAppWindow::unrealize()
{
    mGLArea->make_current();
    try
    {
        mGLArea->throw_if_error();

    }
    catch(const Gdk::GLError& gle)
    {
        std::cerr << "An error occured making the context current during unrealize" << std::endl;
        std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
    }
}

bool CelestiaAppWindow::render(const Glib::RefPtr<Gdk::GLContext>& /* context */)
{
    mCore->tick();
    try
    {
        mGLArea->throw_if_error();
        mCore->draw();
        glFlush();
        return true;
    }
    catch(const Gdk::GLError& gle)
    {
        std::cerr << "An error occurred in the render callback of the GLArea" << std::endl;
        std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
        return false;
    }
}


Gtk::Window* do_builder()
{
    // Load the XML file and instantiate its widgets:
    auto builder = Gtk::Builder::create();
    try
    {
        builder->add_from_resource("/builder/celestia.ui");
    }
    catch (const Glib::Error& error)
    {
        std::cout << "Error loading example_builder.ui: " << error.what() << std::endl;
        return nullptr;
    }

    // Get the GtkBuilder-instantiated window:
    CelestiaAppWindow* pWindow = nullptr;
    builder->get_widget_derived("AppWindow", pWindow);
    if (!pWindow)
    {
        std::cout << "Could not get 'AppWindow' from the builder." << std::endl;
        return nullptr;
    }
    return pWindow;
}

