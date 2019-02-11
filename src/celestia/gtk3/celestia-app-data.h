//
// Created by bizarro on 2/8/19.
//

#ifndef CELESTIA_CELESTIAAPP_H
#define CELESTIA_CELESTIAAPP_H

#include <memory>
#include <celestia/celestiacore.h>

class CelestiaAppData : public CelestiaWatcher  {

public:
    explicit CelestiaAppData(std::shared_ptr<CelestiaCore>);

    void setStartURL(std::string);
    void setFullScreen(bool);
    void setReady(bool);
    bool getReady();
    void initSimulation(ProgressNotifier*);
    void start(double);
    bool initRenderer();
    void setTimeZoneName(std::string);
    void tick();
    void draw();

    std::string getUrl();
    void charEntered(char);
    void openScript(std::string filename);

    void setLabelMode(int, bool);
    void setAmbientLight(float);
    void setVisualMagnitude(float);
    void setGalaxyLightGain(float);
    void setDistanceLimit(int value);
    void setVerbosity(int value);
    void setStarStyle(Renderer::StarStyle value);
    void setTextureResolution(int value);
    void setAltSurface(std::string value);
    void setDefaultRenderFlags();
    void showLocalTime(bool);
    void setVideoSync(bool);
    void setRenderFlags(int);
    void setOrbitMask(int);
    void setLabelMode(int);
    void resize(int, int);

    void resyncLabelActions();

    void notifyChange(CelestiaCore*, int) override;

private:
    std::shared_ptr<CelestiaCore> mCore;
    std::shared_ptr<Renderer> mRenderer;
    std::shared_ptr<Simulation> mSimulation;

    bool mReady {false};
    int mLastX {0};
    int mLastY {0};
    bool mShowLocalTime {false};
    bool mFullScreen {false};
    std::string mStartURL {};

    const float amLevels[3] { 0.0, 0.1, 0.25 };
};


#endif //CELESTIA_CELESTIAAPP_H
