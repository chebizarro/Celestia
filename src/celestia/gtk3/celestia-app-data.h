//
// Created by bizarro on 2/8/19.
//

#ifndef CELESTIA_CELESTIAAPP_H
#define CELESTIA_CELESTIAAPP_H

#include <memory>
#include <celestia/celestiacore.h>

class CelestiaAppData : public CelestiaWatcher {

public:
    CelestiaAppData(std::shared_ptr<CelestiaCore>);

    void setStartURL(std::string);
    void setFullScreen(bool);
    void setReady(bool);
    void initSimulation();
    void start(double);
    bool initRenderer();
    void setTimeZoneName(std::string);
    void tick();
    void draw();

    virtual void notifyChange(CelestiaCore*, int);

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
};


#endif //CELESTIA_CELESTIAAPP_H
