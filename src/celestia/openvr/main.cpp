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

#include <openvr.h>

#if defined(POSIX)
#include "unistd.h"
#endif

#ifndef _WIN32
#define APIENTRY
#endif


class OpenVRApplication {

public:
    OpenVRApplication(int argc, char *argv[]);
    virtual ~OpenVRApplication();

    bool Init();
    bool InitGL();
    bool InitCompositor();
    void ShutDown();
    void RunMainLoop();

private:
    vr::IVRSystem *m_pHMD;
    std::string m_strDriver;
    std::string m_strDisplay;

    CelestiaCore* m_appCore{ nullptr };

};

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
        char buf[1024];
        sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
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

    m_pCompanionWindow = SDL_CreateWindow( "hellovr", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags );
    if (m_pCompanionWindow == NULL)
    {
        printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
    if (m_pContext == NULL)
    {
        printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    glewExperimental = GL_TRUE;
    auto nGlewError = glewInit();
    if (nGlewError != GLEW_OK) {
        printf( "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString( nGlewError ) );
        return false;
    }
    glGetError(); // to clear the error caused deep in GLEW

    if ( SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0 ) {
        printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    m_strDriver = "No Driver";
    m_strDisplay = "No Display";

    m_strDriver = GetTrackedDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
    m_strDisplay = GetTrackedDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

    std::string strWindowTitle = "hellovr - " + m_strDriver + " " + m_strDisplay;
    SDL_SetWindowTitle( m_pCompanionWindow, strWindowTitle.c_str() );

    if (!InitGL()) {
        printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
        return false;
    }

    if (!InitCompositor()) {
        printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
        return false;
    }

    m_appCore = new CelestiaCore();

    return true;
}

bool OpenVRApplication::InitGL() {


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

    OpenVRApplication app(arc, argv);

    if (!app.Init()) {
        app.ShutDown();
        return 1;
    }

    app.RunMainLoop();

    app.ShutDown();

    return 0;

}
