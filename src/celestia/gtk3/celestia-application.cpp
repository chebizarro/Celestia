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

    auto ss = dynamic_cast<ProgressNotifier*>(mSplashScreen.get());

    mAppData->initSimulation(ss);

    get_main_window();

    apply_settings();

    // app menus set up in on_startup()
    // set up context menus

    hide_splash_screen();

    mAppWindow->present();

    mAppData->setReady(true);
}

static int read_labels()
{
    auto settings = Gio::Settings::create("org.celestia.app.labels");
    int lm = Renderer::NoLabels;
    lm |= Renderer::StarLabels  * settings->get_boolean("star");
    lm |= Renderer::PlanetLabels  * settings->get_boolean("planet");
    lm |= Renderer::MoonLabels  * settings->get_boolean("moon");
    lm |= Renderer::ConstellationLabels  * settings->get_boolean("constellation");
    lm |= Renderer::GalaxyLabels  * settings->get_boolean("galaxy");
    lm |= Renderer::AsteroidLabels  * settings->get_boolean("asteroid");
    lm |= Renderer::SpacecraftLabels  * settings->get_boolean("spacecraft");
    lm |= Renderer::LocationLabels  * settings->get_boolean("location");
    lm |= Renderer::CometLabels  * settings->get_boolean("comet");
    lm |= Renderer::NebulaLabels  * settings->get_boolean("nebula");
    lm |= Renderer::OpenClusterLabels  * settings->get_boolean("open-cluster");
    lm |= Renderer::I18nConstellationLabels  * settings->get_boolean("i18n");
    lm |= Renderer::GlobularLabels  * settings->get_boolean("globular");
    return lm;
}

static int read_orbits()
{
    auto settings = Gio::Settings::create("org.celestia.app.orbits");
    int om = 0;
    om |= Body::Planet * settings->get_boolean("planet");
    om |= Body::Moon * settings->get_boolean("moon");
    om |= Body::Asteroid * settings->get_boolean("asteroid");
    om |= Body::Comet * settings->get_boolean("comet");
    om |= Body::Spacecraft * settings->get_boolean("spacecraft");
    om |= Body::Invisible * settings->get_boolean("invisible");
    om |= Body::Unknown * settings->get_boolean("unknown");
    return om;
}

static int read_render_settings()
{
    auto settings = Gio::Settings::create("org.celestia.app.render");

    int rf = Renderer::ShowNothing;
    rf |= Renderer::ShowStars * settings->get_boolean("stars");
    rf |= Renderer::ShowPlanets * settings->get_boolean("planets");
    rf |= Renderer::ShowGalaxies * settings->get_boolean("galaxies");
    rf |= Renderer::ShowDiagrams * settings->get_boolean("diagrams");
    rf |= Renderer::ShowCloudMaps * settings->get_boolean("cloud-maps");
    rf |= Renderer::ShowOrbits * settings->get_boolean("orbits");
    rf |= Renderer::ShowCelestialSphere * settings->get_boolean("celestial-sphere");
    rf |= Renderer::ShowNightMaps * settings->get_boolean("night-maps");
    rf |= Renderer::ShowAtmospheres * settings->get_boolean("atmospheres");
    rf |= Renderer::ShowSmoothLines * settings->get_boolean("smooth-lines");
    rf |= Renderer::ShowEclipseShadows * settings->get_boolean("eclipse-shadows");
    rf |= Renderer::ShowRingShadows * settings->get_boolean("ring-shadows");
    rf |= Renderer::ShowBoundaries * settings->get_boolean("boundaries");
    rf |= Renderer::ShowAutoMag * settings->get_boolean("auto-mag");
    rf |= Renderer::ShowCometTails * settings->get_boolean("comet-tails");
    rf |= Renderer::ShowMarkers * settings->get_boolean("markers");
    rf |= Renderer::ShowPartialTrajectories * settings->get_boolean("partial-trajectories");
    rf |= Renderer::ShowNebulae * settings->get_boolean("nebulae");
    rf |= Renderer::ShowOpenClusters * settings->get_boolean("open-clusters");
    rf |= Renderer::ShowGlobulars * settings->get_boolean("globulars");
    rf |= Renderer::ShowGalacticGrid * settings->get_boolean("grid-galactic");
    rf |= Renderer::ShowEclipticGrid * settings->get_boolean("grid-ecliptic");
    rf |= Renderer::ShowHorizonGrid * settings->get_boolean("grid-horizontal");
    return rf;
}

void CelestiaApplication::apply_settings()
{
    mSettings = Gio::Settings::create("org.celestia.app");
    
    set_sane_win_size(mSettings->get_int("win-width"), mSettings->get_int("win-height"));
    set_sane_win_position(mSettings->get_int("win-x"), mSettings->get_int("win-y"));

    mAppData->setFullScreen(mSettings->get_boolean("full-screen"));
    mAppData->setAmbientLight(static_cast<float>(mSettings->get_double("ambient-light")));
    mAppData->setVisualMagnitude(static_cast<float>(mSettings->get_double("visual-magnitude")));
    mAppData->setGalaxyLightGain(static_cast<float>(mSettings->get_double("galaxy-light-gain")));
    mAppData->setDistanceLimit(mSettings->get_int("distance-limit"));
    mAppData->setVerbosity(mSettings->get_int("verbosity"));
    mAppData->setStarStyle(static_cast<Renderer::StarStyle>(mSettings->get_int("star-style")));
    mAppData->setTextureResolution(mSettings->get_int("texture-resolution"));
    mAppData->setAltSurface(mSettings->get_string("alt-surface-name"));
    mAppData->showLocalTime(mSettings->get_boolean("show-local-time"));
    mAppData->setVideoSync(mSettings->get_boolean("video-sync"));

    mAppData->setRenderFlags(read_render_settings());
    mAppData->setOrbitMask(read_orbits());
    mAppData->setLabelMode(read_labels());

    if(mConfig.fullscreen)
    {
        mAppData->setFullScreen(mConfig.fullscreen);
    }
}

void CelestiaApplication::set_sane_win_size(int width, int height)
{
    auto screen = Gdk::Display::get_default()->get_primary_monitor();
    auto rect = Gdk::Rectangle {};
    screen->get_workarea(rect);

    if (width < 400 || width > rect.get_width() || height < 300 || height > rect.get_height())
    {
        width = 800;
        height = 600;
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
        //auto ss = SplashData { mAppData };

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
        auto _ = mSplashScreen.release();
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

    // File Menu
    add_action("copyurl", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_copy_url));
    add_action("openurl", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_open_url));
    add_action("openscript", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_open_script));
    add_action("captureimage", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_capture_image));
    add_action("capturemovie", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_capture_movie));
    add_action("quit", sigc::mem_fun(*this, &CelestiaApplication::on_menu_file_quit));
    // Navigation Menu
    add_action("selectsol", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_select_sol));
    add_action("tourguide", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_tour_guide));
    add_action("searchobject", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_search_object));
    add_action("gotoobject", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_goto_object));
    add_action("centerselection", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_center_selection));
    add_action("gotoselection", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_goto_selection));
    add_action("followselection", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_follow_selection));
    add_action("syncselection", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_sync_selection));
    add_action("trackselection", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_track_selection));
    add_action("systembrowser", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_system_browser));
    add_action("starbrowser", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_star_browser));
    add_action("eclipsefinder", sigc::mem_fun(*this, &CelestiaApplication::on_menu_nav_eclipse_finder));
    // Time Menu
    // Options Menu
    // Window Menu
    // Help Menu

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

    auto fs = Gtk::FileChooserDialog { "Open Script", Gtk::FILE_CHOOSER_ACTION_OPEN };
    fs.set_transient_for(*mAppWindow.get());
    fs.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    fs.add_button("Open", Gtk::RESPONSE_OK);

    auto result = fs.run();

    if (result == Gtk::RESPONSE_OK)
    {
        auto filename = fs.get_filename();
        mAppData->openScript(filename);
    }

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

void CelestiaApplication::on_menu_nav_tour_guide() {}

void CelestiaApplication::on_menu_nav_search_object() {}

void CelestiaApplication::on_menu_nav_goto_object() {}

void CelestiaApplication::on_menu_nav_center_selection()
{
    mAppData->charEntered('c');
}

void CelestiaApplication::on_menu_nav_goto_selection()
{
    mAppData->charEntered('G');
}

void CelestiaApplication::on_menu_nav_follow_selection()
{
    mAppData->charEntered('F');
}

void CelestiaApplication::on_menu_nav_sync_selection()
{
    mAppData->charEntered('Y');
}

void CelestiaApplication::on_menu_nav_track_selection()
{
    mAppData->charEntered('T');
}

void CelestiaApplication::on_menu_nav_system_browser() {}

void CelestiaApplication::on_menu_nav_star_browser() {}

void CelestiaApplication::on_menu_nav_eclipse_finder() {}
