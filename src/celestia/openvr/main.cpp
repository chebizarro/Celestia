//
// Created by bizarro on 1/29/19.
//

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#if defined( OSX )
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <OpenGL/glu.h>
// Apple's version of glut.h #undef's APIENTRY, redefine it
#define APIENTRY
#else
#include <GL/glu.h>
#endif
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <memory>

#include <openvr.h>

#if defined(POSIX)
#include "unistd.h"
#endif

#ifndef _WIN32
#define APIENTRY
#endif

#include <celestia/celestiacore.h>


class OpenVRApplication {

public:
    OpenVRApplication(int argc, char *argv[]);
    virtual ~OpenVRApplication() = default;

    bool Init();
    bool InitGL();
    bool InitCompositor();
    void ShutDown();
    void RunMainLoop();
    bool HandleInput();
    void RenderFrame();
    void ProcessVREvent(const vr::VREvent_t &event);

private:

    SDL_GLContext m_pContext;
    SDL_Window *m_pCompanionWindow;
    uint32_t m_nCompanionWindowWidth;
    uint32_t m_nCompanionWindowHeight;

    bool m_bVblank;

    vr::IVRSystem *m_pHMD;
    std::string m_strDriver;
    std::string m_strDisplay;

    std::unique_ptr<CelestiaCore> m_appCore;

};

std::string GetTrackedDeviceString( vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = nullptr) {
    auto unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty( unDevice, prop, nullptr, 0, peError );
    if(unRequiredBufferLen == 0)
        return "";

    char *pchBuffer = new char[unRequiredBufferLen];
    vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
    std::string sResult = pchBuffer;
    delete [] pchBuffer;
    return sResult;
}

OpenVRApplication::OpenVRApplication(int argc, char **argv)
        : m_pCompanionWindow(nullptr)
        , m_nCompanionWindowWidth( 640 )
        , m_nCompanionWindowHeight( 320 )
        , m_pHMD(nullptr)
        , m_bVblank( false )
{ }

void OpenVRApplication::ShutDown() {

    if(m_pHMD) {
        vr::VR_Shutdown();
        m_pHMD = nullptr;
    }


    if(m_pCompanionWindow) {
        SDL_DestroyWindow(m_pCompanionWindow);
        m_pCompanionWindow = nullptr;
    }

    SDL_Quit();

}

bool OpenVRApplication::HandleInput() {

    SDL_Event sdlEvent;
    auto bRet = false;

    while (SDL_PollEvent(&sdlEvent) != 0 ) {
        if (sdlEvent.type == SDL_QUIT) {
            bRet = true;
        } else if (sdlEvent.type == SDL_KEYDOWN) {
            if (sdlEvent.key.keysym.sym == SDLK_ESCAPE
                || sdlEvent.key.keysym.sym == SDLK_q) {
                bRet = true;
            }
        }
    }

    // Process SteamVR events
    vr::VREvent_t event;
    while(m_pHMD->PollNextEvent(&event, sizeof(event))) {
        ProcessVREvent(event);
    }

    return bRet;
}

void OpenVRApplication::ProcessVREvent(const vr::VREvent_t &event) {
    switch(event.eventType) {
        case vr::VREvent_TrackedDeviceDeactivated:
            // dprintf( "Device %u detached.\n", event.trackedDeviceIndex );
            break;
        case vr::VREvent_TrackedDeviceUpdated:
            //dprintf( "Device %u updated.\n", event.trackedDeviceIndex );
            break;
    }
}

void OpenVRApplication::RunMainLoop() {

    auto bQuit = false;

    SDL_StartTextInput();
    SDL_ShowCursor(SDL_DISABLE);

    while (!bQuit) {
        bQuit = HandleInput();
        RenderFrame();
    }

    SDL_StopTextInput();
}

void OpenVRApplication::RenderFrame() {
    m_appCore->draw();
}

bool OpenVRApplication::Init() {

    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 ) {
        printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
        return false;
    }

    // Loading the SteamVR Runtime
    vr::EVRInitError eError = vr::VRInitError_None;
    m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

    if ( eError != vr::VRInitError_None ) {
        m_pHMD = nullptr;
        std::string errmsg("Unable to init VR runtime: " + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(eError)));
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", errmsg.c_str(), nullptr);
        return false;
    }

    int nWindowPosX = 700;
    int nWindowPosY = 100;
    Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

    m_pCompanionWindow = SDL_CreateWindow("celestia", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags);
    if (m_pCompanionWindow == nullptr) {
        printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
    if (m_pContext == nullptr) {
        printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    glewExperimental = GL_TRUE;
    auto nGlewError = glewInit();
    if (nGlewError != GLEW_OK) {
        printf( "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString( nGlewError ) );
        return false;
    }
    glGetError();

    if (SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0) {
        printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    m_strDriver = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
    m_strDisplay = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

    std::string strWindowTitle = "Celestia OpenVR - " + m_strDriver + " " + m_strDisplay;
    SDL_SetWindowTitle(m_pCompanionWindow, strWindowTitle.c_str());

    if (!InitGL()) {
        printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
        return false;
    }

    if (!InitCompositor()) {
        printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
        return false;
    }

    m_appCore = std::make_unique<CelestiaCore>();

    return true;
}

bool OpenVRApplication::InitGL() {

    if (!m_appCore->initRenderer()) {
        return false;
    }

    m_appCore->tick();

    return true;
}

bool OpenVRApplication::InitCompositor() {

    if (!vr::VRCompositor()) {
        printf( "Compositor initialization failed. See log file for details\n" );
        return false;
    }

    return true;
}


int main(int argc, char *argv[]) {

    OpenVRApplication app(argc, argv);

    if (!app.Init()) {
        app.ShutDown();
        return 1;
    }

    app.RunMainLoop();

    app.ShutDown();

    return 0;

}
