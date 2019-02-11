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

    mGLArea->set_hexpand(true);
    mGLArea->set_vexpand(true);
    //mGLArea->set_auto_render(true);

    mGLArea->signal_realize().connect(sigc::mem_fun(*this, &CelestiaAppWindow::gl_realize));
    mGLArea->signal_unrealize().connect(sigc::mem_fun(*this, &CelestiaAppWindow::gl_unrealize), false);
    mGLArea->signal_render().connect(sigc::mem_fun(*this, &CelestiaAppWindow::gl_render), false);
    mGLArea->signal_configure_event().connect(sigc::mem_fun(*this, &CelestiaAppWindow::gl_configure));

    //Glib::signal_idle().connect(sigc::mem_fun(*this, &CelestiaAppWindow::gl_idle));
}

bool CelestiaAppWindow::gl_idle()
{
    mAppData->tick();
    gl_render(Glib::RefPtr<Gdk::GLContext>());
}

bool CelestiaAppWindow::gl_configure(GdkEventConfigure *configure_event)
{
    auto w = mGLArea->get_allocated_width();
    auto h = mGLArea->get_allocated_height();
    mAppData->resize(w, h);
    return false;
}

void CelestiaAppWindow::gl_realize()
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

        //mGLArea->set_has_depth_buffer();

        if (!mAppData->initRenderer())
        {
            std::cerr << "Failed to initialize renderer.\n";
        }

        mAppData->tick();

        mAppData->start((double)time(nullptr) / 86400.0 + (double)astro::Date(1970, 1, 1));
        mAppData->setTimeZoneName("UTC");

        auto glcontext = mGLArea->get_context();
        auto glwindow = glcontext->get_window();
        auto frame_clock = gdk_window_get_frame_clock(glwindow->gobj());

        g_signal_connect_swapped(frame_clock
            ,"update"
            ,G_CALLBACK(gtk_gl_area_queue_render)
            ,mGLArea->gobj()
            );


        gdk_frame_clock_begin_updating(frame_clock);

    }
    catch(const Gdk::GLError& gle)
    {
        std::cerr << "An error occured making the context current during realize:" << std::endl;
        std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
    }
}

void CelestiaAppWindow::gl_unrealize()
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

bool CelestiaAppWindow::gl_render(const Glib::RefPtr<Gdk::GLContext>& context)
{
    glClearColor (0.5, 0.5, 0.5, 1.0);
    glClear (GL_COLOR_BUFFER_BIT);

    try
    {
        if (mAppData->getReady()) {
            mAppData->tick();
            mGLArea->throw_if_error();
            mAppData->draw();

            //glFlush();
        }
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

