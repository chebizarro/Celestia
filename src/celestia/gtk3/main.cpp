#include "celestia-application.h"

int main(int argc, char* argv[]) {

    auto application = CelestiaApplication::create();

    return application->run(argc, argv);
}