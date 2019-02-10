#ifndef CELESTIA_CELESTIA_APPLICATION_H
#define CELESTIA_CELESTIA_APPLICATION_H

#include <gtkmm.h>
#include "celestia-app-data.h"
#include "celestia-splash-window.h"

class CelestiaAppWindow;

class CelestiaApplication: public Gtk::Application
{
protected:
    CelestiaApplication();

public:
    static Glib::RefPtr<CelestiaApplication> create();

protected:
    void on_activate() override;
    int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;
    int on_handle_local_options(const Glib::RefPtr<Glib::VariantDict>& options);


private:
    void show_splash_screen();
    void hide_splash_screen();
    void get_main_window();
    void apply_settings();
    void set_sane_win_size(int, int);
    void set_sane_win_position(int, int);

    Glib::RefPtr<CelestiaAppWindow> mAppWindow;
    Glib::RefPtr<CelestiaSplashWindow> mSplashScreen;
    Glib::RefPtr<Gio::Settings> mSettings;

    std::shared_ptr<CelestiaAppData> mAppData;

    struct Config {
        std::string conf {};
        std::string dir {};
        bool fullscreen {false};
        bool nosplash {false};
        std::vector<std::string> extrasdir;
    };

    Config mConfig;
};


#endif //CELESTIA_CELESTIA_APPLICATION_H
