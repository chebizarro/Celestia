//
// Created by bizarro on 2/8/19.
//

#include <celestia/url.h>
#include <celutil/filetype.h>
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

std::string CelestiaAppData::getUrl()
{
    CelestiaState appState;
    appState.captureState(mCore.get());
    auto url = Url { appState, Url::CurrentVersion };
    return  url.getAsString();
}

void CelestiaAppData::charEntered(const char c)
{
    mCore->charEntered(c);
}

void CelestiaAppData::openScript(std::string filename)
{
    ContentType type = DetermineFileType(filename);

    if (type == Content_CelestiaScript)
    {
        mCore->runScript(filename);
    }
    else if (type == Content_CelestiaLegacyScript)
    {

    }
}


void CelestiaAppData::setLabelMode(int mode, bool state)
{
    auto lm = (mRenderer->getLabelMode() & ~mode) | (state ? mode : 0);
    mRenderer->setLabelMode(lm);
}

void CelestiaAppData::resyncLabelActions() {

    auto actionName = std::string {};
    auto f = mRenderer->getLabelMode();

    for (int i = Renderer::StarLabels; i <= Renderer::GlobularLabels; i *= 2)
    {
        switch (i)
        {
            case Renderer::StarLabels: actionName = "LabelStars"; break;
            case Renderer::PlanetLabels: actionName = "LabelPlanets"; break;
            case Renderer::DwarfPlanetLabels: actionName = "LabelDwarfPlanets"; break;
            case Renderer::MoonLabels: actionName = "LabelMoons"; break;
            case Renderer::MinorMoonLabels: actionName = "LabelMinorMoons"; break;
            case Renderer::ConstellationLabels: actionName = "LabelConstellations"; break;
            case Renderer::GalaxyLabels: actionName = "LabelGalaxies"; break;
            case Renderer::AsteroidLabels: actionName = "LabelAsteroids"; break;
            case Renderer::SpacecraftLabels: actionName = "LabelSpacecraft"; break;
            case Renderer::LocationLabels: actionName = "LabelLocations"; break;
            case Renderer::CometLabels: actionName = "LabelComets"; break;
            case Renderer::NebulaLabels: actionName = "LabelNebulae"; break;
            case Renderer::OpenClusterLabels: actionName = "LabelOpenClusters"; break;
            case Renderer::GlobularLabels: actionName = "LabelGlobulars"; break;
            case Renderer::I18nConstellationLabels: /* Not used yet */
            default: actionName = "";
        }

        if (!actionName.empty())
        {
            /* Get the action */
            //action = gtk_action_group_get_action(app->agLabel, actionName);

            /* The current i anded with the labelMode gives state of flag */
            //gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), (i & f));
        }
    }
}

void CelestiaAppData::notifyChange(CelestiaCore*, int property)
{
    if (property & CelestiaCore::LabelFlagsChanged)
    {
        resyncLabelActions();
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