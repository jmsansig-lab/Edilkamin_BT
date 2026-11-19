#pragma once

// Over-the-air update page (HTTP, local network only).
//   GET  /        status page (version, uptime, free heap)
//   GET  /update  upload form
//   POST /update  firmware.bin upload, then reboot
// Both update routes require HTTP basic auth: user "admin", password = the
// MQTT password entered in the configuration portal ("edilkamin" if empty).
// The Update library validates the image header and the partition size; an
// interrupted or invalid upload leaves the running firmware untouched.

#include <Arduino.h>
#include <Update.h>
#include <WebServer.h>

namespace WebUpdate {

inline WebServer &server() {
  static WebServer instance(80);
  return instance;
}

inline const char *&password() {
  static const char *value = "edilkamin";
  return value;
}

inline bool &uploadOk() {
  static bool value = false;
  return value;
}

inline bool authorized() {
  if (server().authenticate("admin", password())) return true;
  server().requestAuthentication(BASIC_AUTH, "Edilkamin BT Bridge");
  return false;
}

inline void sendPage(int code, const String &body) {
  String html = F("<!DOCTYPE html><html lang=\"fr\"><head><meta charset=\"utf-8\">"
                  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                  "<title>Edilkamin BT Bridge</title><style>"
                  "body{font-family:system-ui,sans-serif;max-width:560px;margin:2em auto;padding:0 1em;color:#222}"
                  "h1{font-size:1.4em}code{background:#eee;padding:2px 4px;border-radius:3px}"
                  "button{padding:.6em 1.2em;font-size:1em}.warn{color:#a40}</style></head><body>"
                  "<h1>Edilkamin BT Bridge</h1>");
  html += body;
  html += F("</body></html>");
  server().send(code, "text/html; charset=utf-8", html);
}

inline void begin(const char *version, const char *pass) {
  if (pass && pass[0]) password() = pass;
  WebServer &web = server();

  web.on("/", HTTP_GET, [version]() {
    String body = F("<p>Version : <code>");
    body += version;
    body += F("</code></p><p>Uptime : ");
    body += String(millis() / 60000UL);
    body += F(" min &middot; heap libre : ");
    body += String(ESP.getFreeHeap());
    body += F(" octets</p><p><a href=\"/update\">Mise &agrave; jour du firmware</a></p>");
    sendPage(200, body);
  });

  web.on("/update", HTTP_GET, []() {
    if (!authorized()) return;
    sendPage(200, F("<p>S&eacute;lectionne le fichier <code>firmware.bin</code> "
                    "(pas le <code>merged-firmware.bin</code>).</p>"
                    "<form method=\"POST\" action=\"/update\" enctype=\"multipart/form-data\">"
                    "<input type=\"file\" name=\"firmware\" accept=\".bin\" required> "
                    "<button type=\"submit\">Envoyer</button></form>"
                    "<p class=\"warn\">Ne coupe pas l'alimentation pendant l'envoi "
                    "(environ 30 s). Le module red&eacute;marre ensuite tout seul.</p>"));
  });

  web.on(
      "/update", HTTP_POST,
      []() {
        if (!authorized()) return;
        if (uploadOk()) {
          sendPage(200, F("<p>Mise &agrave; jour r&eacute;ussie. Red&eacute;marrage&hellip;</p>"
                          "<p><a href=\"/\">Retour</a> (dans ~20 s)</p>"));
          delay(500);
          ESP.restart();
        } else {
          String body = F("<p class=\"warn\">&Eacute;chec de la mise &agrave; jour : ");
          body += Update.errorString();
          body += F(". Le firmware actuel reste en place.</p><p><a href=\"/update\">R&eacute;essayer</a></p>");
          sendPage(500, body);
        }
      },
      []() {
        HTTPUpload &upload = server().upload();
        if (upload.status == UPLOAD_FILE_START) {
          uploadOk() = false;
          // Authentication is checked before any byte is written to flash.
          if (!server().authenticate("admin", password())) return;
          Serial.printf("OTA start: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) Update.printError(Serial);
        } else if (upload.status == UPLOAD_FILE_WRITE) {
          if (Update.isRunning() &&
              Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            Update.printError(Serial);
        } else if (upload.status == UPLOAD_FILE_END) {
          if (Update.isRunning()) {
            uploadOk() = Update.end(true) && !Update.hasError();
            if (!uploadOk()) Update.printError(Serial);
            Serial.printf("OTA end: %u bytes, ok=%d\n", upload.totalSize, uploadOk());
          }
        } else if (upload.status == UPLOAD_FILE_ABORTED) {
          if (Update.isRunning()) Update.abort();
          uploadOk() = false;
        }
      });

  web.onNotFound([]() { server().send(404, "text/plain", "Not found"); });
  web.begin();
}

inline void handle() { server().handleClient(); }

}  // namespace WebUpdate
