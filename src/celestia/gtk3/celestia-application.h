#ifndef CELESTIA_CELESTIA_APPLICATION_H
#define CELESTIA_CELESTIA_APPLICATION_H

#include <gtkmm.h>

class CelestiaAppWindow;

class CelestiaApplication: public Gtk::Application
{
protected:
    CelestiaApplication();

public:
    static Glib::RefPtr<CelestiaApplication> create();

protected:
    // Override default signal handlers:
    void on_activate() override;
    void on_open(const Gio::Application::type_vec_files& files,
                 const Glib::ustring& hint) override;

private:
    CelestiaAppWindow* create_appwindow();
    void on_hide_window(Gtk::Window* window);
};


#endif //CELESTIA_CELESTIA_APPLICATION_H
