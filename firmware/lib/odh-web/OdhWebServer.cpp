#include "OdhWebServer.h"

#ifndef NATIVE_SIM
#include <WiFi.h>
#endif

namespace odh {

OdhWebServer::OdhWebServer(uint16_t port)
    : _port(port),
      _server(port) {}

bool OdhWebServer::begin(const char *ssid, const char *password, uint16_t port) {
    _port = port;

#ifndef NATIVE_SIM
    // Mount LittleFS for static file serving.
    if (!LittleFS.begin(true)) {
        // Format on first mount failure.
        LittleFS.begin(true);
    }

    // Start WiFi AP.
    WiFi.softAP(ssid, password);
    delay(100); // Allow AP to stabilise.
#else
    (void)ssid;
    (void)password;
#endif

    _server.begin();
    _running = true;
    return true;
}

void OdhWebServer::stop() {
    _server.end();
#ifndef NATIVE_SIM
    WiFi.softAPdisconnect(true);
#endif
    _running = false;
}

#ifndef NATIVE_SIM
void OdhWebServer::serveStatic(const char *uri, const char *fsPath, const char *defaultFile) {
    _server.serveStatic(uri, LittleFS, fsPath).setDefaultFile(defaultFile);
}
#else
void OdhWebServer::serveStatic(const char *uri, const char *fsPath, const char *defaultFile) {
    _server.serveStatic(uri, nullptr, fsPath).setDefaultFile(defaultFile);
}
#endif

} // namespace odh
