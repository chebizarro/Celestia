#include "celestia-application.h"

int main(int argc, char* argv[]) {

    auto application = CelestiaApplication::create();

    return application->run(argc, argv);
}

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
    return main(__argc, __argv);
}
#endif /* WIN32 */