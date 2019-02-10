//
// Created by bizarro on 2/6/19.
//

#include "celestia-appwindow.h"
#include "celestia-app-data.h"
#include <gtk/gtk.h>
#include <iostream>
#include <GL/glew.h>

CelestiaAppWindow::CelestiaAppWindow()
        : Gtk::ApplicationWindow()
{
}


CelestiaAppWindow::CelestiaAppWindow(_GtkApplicationWindow*& win, Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(win),
    mBuilder(builder)
{
    Gtk::GLArea* pGLArea = nullptr;
    builder->get_widget("GLArea", pGLArea);
    mGLArea = Glib::RefPtr<Gtk::GLArea>(pGLArea);

    mGLArea->signal_realize().connect(sigc::mem_fun(*this, &CelestiaAppWindow::realize));
    mGLArea->signal_unrealize().connect(sigc::mem_fun(*this, &CelestiaAppWindow::unrealize), false);
    mGLArea->signal_render().connect(sigc::mem_fun(*this, &CelestiaAppWindow::render), false);

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

        if (!mAppData->initRenderer())
        {
            std::cerr << "Failed to initialize renderer.\n";
        }

        mAppData->start((double)time(nullptr) / 86400.0 + (double)astro::Date(1970, 1, 1));
        mAppData->setTimeZoneName("UTC");

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
    mAppData->tick();
    try
    {
        mGLArea->throw_if_error();
        mAppData->draw();
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

void CelestiaAppWindow::set_app_data(std::shared_ptr<CelestiaAppData> _app)
{
    mAppData = std::move(_app);
}

Glib::RefPtr<CelestiaAppWindow> CelestiaAppWindow::create(std::shared_ptr<CelestiaAppData> app)
{
    auto builder = Gtk::Builder::create();
    try
    {
        builder->add_from_resource("/builder/celestia.ui");
    }
    catch (const Glib::Error& error)
    {
        std::cout << "Error loading example_builder.ui: " << error.what() << std::endl;
        return Glib::RefPtr<CelestiaAppWindow>();
    }

    CelestiaAppWindow* pWindow = nullptr;
    builder->get_widget_derived("AppWindow", pWindow);
    if (!pWindow)
    {
        std::cout << "Could not get 'AppWindow' from the builder." << std::endl;
        return Glib::RefPtr<CelestiaAppWindow>();
    }

    pWindow->set_app_data(std::move(app));

    return Glib::RefPtr<CelestiaAppWindow>(pWindow);
}

