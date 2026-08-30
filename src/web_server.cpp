#define WIN32_LEAN_AND_MEAN
#include "../include/web_server.h"
#include "../src/vendor/httplib.h"
#include "../include/player_controller.h"
#include <iostream>

FrameZero::PlayerController* g_p1 = nullptr;
std::atomic<bool> g_serverRunning{true};

void RunWebServer() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        FILE* f = fopen("web_ui/index.html", "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            size_t size = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* buf = new char[size];
            fread(buf, 1, size, f);
            fclose(f);
            res.set_content(std::string(buf, size), "text/html");
            delete[] buf;
        } else {
            res.set_content("<h1>Web UI not found!</h1>", "text/html");
        }
    });

    svr.Get("/api/update", [](const httplib::Request& req, httplib::Response& res) {
        if (!g_p1) return;
        if (req.has_param("var") && req.has_param("val")) {
            std::string var = req.get_param_value("var");
            float val = std::stof(req.get_param_value("val"));
            if (var == "walkSpeed") g_p1->walkSpeed = FrameZero::Fixed(val);
            else if (var == "jumpForce") g_p1->jumpForce = FrameZero::Fixed(val);
            else if (var == "hitboxDamage") g_p1->attackHitbox.damage = FrameZero::Fixed(val);
            res.set_content("OK", "text/plain");
        }
    });

    std::cout << "Web Engine UI running at http://localhost:8080" << std::endl;
    svr.listen("localhost", 8080);
}
