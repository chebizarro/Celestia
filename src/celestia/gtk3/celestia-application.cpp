//
// Created by Chris Daley on 2/6/19.
//

#include "celestia-application.h"
#include "celestia-appwindow.h"
#include "celestia-splash-window.h"

struct SplashData {
    std::shared_ptr<CelestiaAppData> app;
    bool redraw { true };
    std::string text { "Initializing..." };
};

CelestiaApplication::CelestiaApplication()
        : Gtk::Application("org.celestia.application", Gio::APPLICATION_FLAGS_NONE)
{
    // Add additional command-line arguments
    add_main_option_entry(Gio::Application::OptionType::OPTION_TYPE_FILENAME, "conf", 'c', "Alternate configuration file", "file");
    add_main_option_entry(Gio::Application::OptionType::OPTION_TYPE_FILENAME, "dir", 'd', "Alternate installation directory", "directory");
    add_main_option_entry(Gio::Application::OptionType::OPTION_TYPE_BOOL, "fullscreen", 'f', "Start full-screen");
    add_main_option_entry(Gio::Application::OptionType::OPTION_TYPE_BOOL, "nosplash", 's', "Disable splash screen");
    add_main_option_entry(Gio::Application::OptionType::OPTION_TYPE_FILENAME_VECTOR, "extrasdir", 'e', "Additional \"extras\" directory", "directory");
}

void CelestiaApplication::on_activate()
{
    show_splash_screen();

    auto core = std::make_shared<CelestiaCore>();

    mAppData = std::make_shared<CelestiaAppData>(core);

    mAppData->initSimulation();

    get_main_window();

    // get and apply settings
    apply_settings();

    if(mConfig.fullscreen)
    {
        mAppData->setFullScreen(mConfig.fullscreen);
    }

    // set up app menus
    // set up context menus

    hide_splash_screen();

    mAppWindow->present();

    mAppData->setReady(true);

}

void CelestiaApplication::apply_settings()
{
    mSettings = Gio::Settings::create("org.celestia.app");

    mAppData->setFullScreen(mSettings->get_boolean("full-screen"));

    set_sane_win_size(mSettings->get_int("win-width"), mSettings->get_int("win-height"));

    set_sane_win_position(mSettings->get_int("win-x"), mSettings->get_int("win-y"));

}

void CelestiaApplication::set_sane_win_size(int width, int height)
{
    auto screen = Gdk::Display::get_default()->get_primary_monitor();
    auto rect = Gdk::Rectangle {};
    screen->get_workarea(rect);

    if (width < 320 || width > rect.get_width() || height < 240 || height > rect.get_height())
    {
        width = 640;
        height = 480;
    }

    mAppWindow->set_size_request(width, height);
}

void CelestiaApplication::set_sane_win_position(int x, int y)
{
    auto screen = Gdk::Display::get_default()->get_primary_monitor();
    auto rect = Gdk::Rectangle {};
    screen->get_workarea(rect);

    if (x > 0 && x < rect.get_width() && y > 0 && y < rect.get_height())
    {
        mAppWindow->move(x, y);
    }

}

void CelestiaApplication::get_main_window()
{
    mAppWindow = CelestiaAppWindow::create(mAppData);
    add_window(*mAppWindow.get());
}

void CelestiaApplication::show_splash_screen()
{
    if(!mConfig.nosplash) {
        auto ss = SplashData { mAppData };

        mSplashScreen = CelestiaSplashWindow::create();
        mSplashScreen->present();

        while (Gtk::Main::events_pending()) {
            Gtk::Main::iteration();
        }
    }
}

void CelestiaApplication::hide_splash_screen()
{
    if(!mConfig.nosplash) {

        mSplashScreen->hide();

    }
}

template <typename T_ArgType>
static bool get_arg_value(const Glib::RefPtr<Glib::VariantDict>& options, const Glib::ustring& arg_name, T_ArgType& arg_value)
{
    arg_value = T_ArgType();
    if(options->lookup_value(arg_name, arg_value))
    {
        //std::cout << "The \"" << arg_name << "\" value was in the options VariantDict." << std::endl;
        return true;
    }
    else
    {
        auto gvariant = g_variant_dict_lookup_value(options->gobj(), arg_name.c_str(), nullptr);
        if(!gvariant)
        {
            std::cerr << "The \"" << arg_name << "\" value was not in the options VariantDict." << std::endl;
        }
        else
        {
            std::cerr <<
                      "The \"" << arg_name <<"\" value was of type " << g_variant_get_type_string(gvariant) <<
                      " instead of " << Glib::Variant<T_ArgType>::variant_type().get_string() << std::endl;
        }
    }

    return false;
}


int CelestiaApplication::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine> &command_line)
{
    const auto options = command_line->get_options_dict();
    if(!options)
        std::cerr << G_STRFUNC << ": options is null." << std::endl;

    get_arg_value(options, "conf", mConfig.conf);
    get_arg_value(options, "dir", mConfig.dir);
    get_arg_value(options, "fullscrean", mConfig.fullscreen);
    get_arg_value(options, "nosplash", mConfig.nosplash);
    get_arg_value(options, G_OPTION_REMAINING, mConfig.extrasdir);

    activate();

    return EXIT_SUCCESS;
}

int CelestiaApplication::on_handle_local_options(const Glib::RefPtr<Glib::VariantDict> &options)
{
    if(!options)
        std::cerr << G_STRFUNC << ": options is null." << std::endl;

    get_arg_value(options, "conf", mConfig.conf);
    get_arg_value(options, "dir", mConfig.dir);
    get_arg_value(options, "fullscrean", mConfig.fullscreen);
    get_arg_value(options, "nosplash", mConfig.nosplash);
    get_arg_value(options, G_OPTION_REMAINING, mConfig.extrasdir);

    return -1;
}

Glib::RefPtr<CelestiaApplication> CelestiaApplication::create()
{
    return Glib::RefPtr<CelestiaApplication>(new CelestiaApplication());
}

void CelestiaApplication::on_startup()
{
    Gtk::Application::on_startup();

    mBuilder = Gtk::Builder::create();

    add_action("copyurl", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_copy_url));
    add_action("openurl", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_open_url));
    add_action("openscript", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_open_script));
    add_action("captureimage", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_capture_image));
    add_action("capturemovie", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_capture_movie));
    add_action("quit", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_quit));

    add_action("selectsol", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_select_sol));


    try
    {
        mBuilder->add_from_resource("/builder/menus.ui");
    }
    catch (const Glib::Error& ex)
    {
        std::cerr << "Building menus failed: " << ex.what();
    }

    auto object = mBuilder->get_object("MainMenu");
    auto menuBar = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);

    if(menuBar)
    {
        set_menubar(menuBar);
    }

}

void CelestiaApplication::on_menu_file_copy_url()
{
    auto cb = Gtk::Clipboard::get();
    cb->set_text(mAppData->getUrl());
}

void CelestiaApplication::on_menu_file_open_url()
{

}

void CelestiaApplication::on_menu_file_open_script()
{

}

void CelestiaApplication::on_menu_file_capture_image()
{

}

void CelestiaApplication::on_menu_file_capture_movie()
{

}

void CelestiaApplication::on_menu_file_quit()
{
    quit();
}


void CelestiaApplication::on_menu_nav_select_sol()
{
    mAppData->charEntered('H');
}
