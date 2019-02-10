//
// Created by bizarro on 2/8/19.
//

#include "celestia-app-data.h"

CelestiaAppData::CelestiaAppData(std::shared_ptr<CelestiaCore> core)
: CelestiaWatcher(*core.get())
, mCore(std::move(core))
{
}

void CelestiaAppData::setStartURL(std::string url)
{
    mStartURL = std::move(url);
}

void CelestiaAppData::setReady(bool ready)
{
    mReady = ready;
}

void CelestiaAppData::setFullScreen(bool fullscreen)
{
    mFullScreen = fullscreen;
}

void CelestiaAppData::start(double _time)
{
    mCore->start(_time);
}

bool CelestiaAppData::initRenderer()
{
    mCore->initRenderer();
}

void CelestiaAppData::setTimeZoneName(std::string _tz)
{
    mCore->setTimeZoneName(_tz);
}

void CelestiaAppData::tick()
{
    mCore->tick();
}

void CelestiaAppData::draw()
{
    mCore->draw();
}

void CelestiaAppData::initSimulation()
{
    vector<std::string> configDirs;
    mCore->initSimulation("", configDirs, nullptr);

    mSimulation = std::shared_ptr<Simulation>(mCore->getSimulation());
    mRenderer = std::shared_ptr<Renderer>(mCore->getRenderer());

    mRenderer->setSolarSystemMaxDistance(mCore->getConfig()->SolarSystemMaxDistance);
}

void CelestiaAppData::notifyChange(CelestiaCore*, int property)
{
    if (property & CelestiaCore::LabelFlagsChanged)
    {
        // resyncLabelActions(app);
    }

    else if (property & CelestiaCore::RenderFlagsChanged)
    {
        /*
        resyncRenderActions(app);
        resyncOrbitActions(app);
        resyncStarStyleActions(app);
        resyncTextureResolutionActions(app);
        */
    }

        /*
        else if (property & CelestiaCore::VerbosityLevelChanged)
            resyncVerbosityActions(app);

        else if (property & CelestiaCore::TimeZoneChanged)
            resyncTimeZoneAction(app);

        else if (property & CelestiaCore::AmbientLightChanged)
            resyncAmbientActions(app);
        */

    else if (property == CelestiaCore::TextEnterModeChanged)
    {
        if (mCore->getTextEnterMode() != 0)
        {
            /* Grey-out the menu */
            //gtk_widget_set_sensitive(app->mainMenu, FALSE);

            /* Disable any actions that will interfere in typing and
               autocomplete
            gtk_action_group_set_sensitive(app->agMain, FALSE);
            gtk_action_group_set_sensitive(app->agRender, FALSE);
            gtk_action_group_set_sensitive(app->agLabel, FALSE);
             */
        }
        else
        {
            /* Set the menu normal */
            //gtk_widget_set_sensitive(app->mainMenu, TRUE);

            /* Re-enable action groups
            gtk_action_group_set_sensitive(app->agMain, TRUE);
            gtk_action_group_set_sensitive(app->agRender, TRUE);
            gtk_action_group_set_sensitive(app->agLabel, TRUE);
              */
        }
    }

    else if (property & CelestiaCore::GalaxyLightGainChanged)
    {
        // resyncGalaxyGainActions(app);

    }
}