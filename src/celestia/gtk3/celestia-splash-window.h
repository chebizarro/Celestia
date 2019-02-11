//
// Created by bizarro on 2/9/19.
//

#ifndef CELESTIA_CELESTIASPLASHWINDOW_H
#define CELESTIA_CELESTIASPLASHWINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/label.h>
#include <celestia/celestiacore.h>

class CelestiaSplashWindow : public Gtk::Window, public ProgressNotifier {

public:
    CelestiaSplashWindow();
    CelestiaSplashWindow(_GtkWindow*&, Glib::RefPtr<Gtk::Builder>&);
    static Glib::RefPtr<CelestiaSplashWindow> create();

    void update(const std::string&) override;

protected:
    void screen_changed(const Glib::RefPtr<Gdk::Screen>& scr);
    bool on_configure_event(GdkEventConfigure *configure_event) override;

private:
    void setText(const char*);

    Glib::RefPtr<Gtk::Label> mLabel;
    Glib::RefPtr<Gtk::Image> mImage;
    Glib::RefPtr<Gtk::Fixed> mFixed;
};


#endif //CELESTIA_CELESTIASPLASHWINDOW_H
