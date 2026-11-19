#pragma once
#include "DiagnosticDiscovery.h"
#include "RegisterScan.h"
#include <ArduinoJson.h>
#include <stdio.h>

// Home Assistant discovery for the read-only exploration mode.
namespace ScanDiscovery {
constexpr const char *SwitchConfigTopic =
    "homeassistant/switch/edilkamin_322707E4_scan/config";
constexpr const char *CommandTopic = "edilkamin/322707E4/scan/set";
constexpr const char *StateTopic = "edilkamin/322707E4/scan/state";
constexpr const char *ProgressConfigTopic =
    "homeassistant/sensor/edilkamin_322707E4_scan_progress/config";
constexpr const char *ProgressTopic = "edilkamin/322707E4/scan_progress/state";

inline void topic(char *out, size_t size, unsigned index, const char *suffix) {
  char key[16];
  RegisterScan::key(key, sizeof(key), index);
  snprintf(out, size, "edilkamin/322707E4/%s/%s", key, suffix);
}
inline void configTopic(char *out, size_t size, unsigned index) {
  char key[16];
  RegisterScan::key(key, sizeof(key), index);
  snprintf(out, size, "homeassistant/sensor/edilkamin_322707E4_%s/config", key);
}

inline void chunk(JsonDocument &config, unsigned index) {
  config.clear();
  uint16_t reg = 0;
  RegisterScan::chunkStart(index, reg);
  char buffer[120];
  snprintf(buffer, sizeof(buffer), "Exploration 0x%04X", reg);
  config["name"] = buffer;
  char key[16];
  RegisterScan::key(key, sizeof(key), index);
  snprintf(buffer, sizeof(buffer), "322707E4_edilkamin_%s", key);
  config["unique_id"] = buffer;
  topic(buffer, sizeof(buffer), index, "state");
  config["state_topic"] = buffer;
  config["icon"] = "mdi:magnify-scan";
  DiagnosticDiscovery::device(config);
  DiagnosticDiscovery::availability(config);
}

inline void scanSwitch(JsonDocument &config) {
  config.clear();
  config["name"] = "Mode exploration";
  config["unique_id"] = "322707E4_edilkamin_scan";
  config["command_topic"] = CommandTopic;
  config["state_topic"] = StateTopic;
  config["payload_on"] = "ON";
  config["payload_off"] = "OFF";
  config["retain"] = false;
  config["icon"] = "mdi:magnify-scan";
  DiagnosticDiscovery::device(config);
  DiagnosticDiscovery::availability(config);
}

inline void progress(JsonDocument &config) {
  config.clear();
  config["name"] = "Exploration progression";
  config["unique_id"] = "322707E4_edilkamin_scan_progress";
  config["state_topic"] = ProgressTopic;
  config["icon"] = "mdi:progress-clock";
  DiagnosticDiscovery::device(config);
  DiagnosticDiscovery::availability(config);
}
} // namespace ScanDiscovery
