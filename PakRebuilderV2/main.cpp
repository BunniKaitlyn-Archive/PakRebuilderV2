#include "app.h"

pak_rebuilder::app* g_App = nullptr;

int main() {
    if (g_App = new pak_rebuilder::app) {
        g_App->start();
        return g_App->exit_code;
    }
    return 0;
}
