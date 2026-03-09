/**
 * WebServer – ESPAsyncWebServer + LittleFS wrapper.
 *
 * Provides a thin abstraction for starting an async web server that
 * serves static files from LittleFS and supports REST API routes.
 */

#pragma once

#include <ESPAsyncWebServer.h>
#include <cstdint>
#include <functional>

#ifndef NATIVE_SIM
#include <LittleFS.h>
#endif

namespace odh {

/**
 * Thin wrapper around ESPAsyncWebServer with LittleFS support.
 *
 * Usage:
 *   WebServer server;
 *   server.begin("MyAP", "password", 80);
 *   server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
 *   server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) { ... });
 */
class OdhWebServer {
public:
    explicit OdhWebServer(uint16_t port = 80);
    ~OdhWebServer() = default;

    // Non-copyable.
    OdhWebServer(const OdhWebServer &)            = delete;
    OdhWebServer &operator=(const OdhWebServer &) = delete;

    /**
     * Start WiFi AP and HTTP server.
     *
     * @param ssid       AP SSID.
     * @param password   AP password (empty for open).
     * @param port       HTTP port.
     * @return true on success.
     */
    bool begin(const char *ssid, const char *password, uint16_t port = 80);

    /// Stop the server and WiFi AP.
    void stop();

    /// True if the server is running.
    bool isRunning() const {
        return _running;
    }

    /// Direct access to the underlying AsyncWebServer.
    AsyncWebServer &server() {
        return _server;
    }

    /**
     * Convenience: add a request handler.
     */
    void on(const char *uri, WebRequestMethodComposite method, ArRequestHandlerFunction handler) {
        _server.on(uri, method, handler);
    }

    /**
     * Convenience: add a body request handler (for JSON POST/PUT).
     */
    void onBody(const char *uri, WebRequestMethodComposite method, ArRequestHandlerFunction handler, ArBodyHandlerFunction bodyHandler) {
        _server.on(uri, method, handler, nullptr, bodyHandler);
    }

    /**
     * Serve static files from LittleFS.
     */
    void serveStatic(const char *uri, const char *fsPath, const char *defaultFile = "index.html");

private:
    uint16_t _port;
    bool _running = false;

    AsyncWebServer _server;
};

} // namespace odh
