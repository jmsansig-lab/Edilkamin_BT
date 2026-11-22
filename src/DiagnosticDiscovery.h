#pragma once
#include "Diagnostics.h"
#include "TechMenu.h"
#include <ArduinoJson.h>
#include <stdio.h>
#include <string.h>

namespace DiagnosticDiscovery {
inline void topic(char *out, size_t size, unsigned id, const char *suffix) {
  snprintf(out, size, "edilkamin/322707E4/%s/%s", Diagnostics::fields[id].key, suffix);
}
inline void configTopic(char *out, size_t size, unsigned id) {
  snprintf(out, size, "homeassistant/sensor/edilkamin_322707E4_%s/config",
           Diagnostics::fields[id].key);
}
inline void availability(JsonDocument &config, const char *dataTopic = nullptr) {
  config["availability_mode"] = "all";
  JsonArray list = config["availability"].to<JsonArray>();
  JsonObject bridge = list.add<JsonObject>();
  bridge["topic"] = "edilkamin/322707E4/bridge/availability";
  bridge["payload_available"] = "ONLINE";
  bridge["payload_not_available"] = "OFFLINE";
  if (dataTopic) {
    JsonObject data = list.add<JsonObject>();
    data["topic"] = dataTopic;
    data["payload_available"] = "ONLINE";
    data["payload_not_available"] = "OFFLINE";
  }
}
inline void device(JsonDocument &config) {
  config["device"]["identifiers"].to<JsonArray>().add("322707E4");
  config["entity_category"] = "diagnostic";
}
inline void build(JsonDocument &config, unsigned id) {
  config.clear();
  const Diagnostics::Field &f = Diagnostics::fields[id];
  char buffer[120];
  config["name"] = f.name;
  snprintf(buffer, sizeof(buffer), "322707E4_edilkamin_%s", f.key);
  config["unique_id"] = buffer;
  topic(buffer, sizeof(buffer), id, "state");
  config["state_topic"] = buffer;
  device(config);
  if (f.unit) {
    config["unit_of_measurement"] = f.unit;
    if (!strcmp(f.unit, "h")) {
      config["device_class"] = "duration";
      config["state_class"] = "total_increasing";
    } else if (!strcmp(f.unit, "s")) {
      config["device_class"] = "duration";
      config["state_class"] = "measurement";
    } else {
      // Live combustion values: regular sensors, not diagnostics.
      config.remove("entity_category");
      config["state_class"] = "measurement";
      if (!strcmp(f.unit, "\u00b0C")) config["device_class"] = "temperature";
      else if (!strcmp(f.unit, "rpm")) config["icon"] = "mdi:fan";
      else config["icon"] = "mdi:gauge";
    }
  }
  topic(buffer, sizeof(buffer), id, "availability");
  availability(config, buffer);
}
// Writable settings: entity_category "config". The state and availability
// topics are the ones of the matching diagnostic (single source of truth).
inline void setting(JsonDocument &config, unsigned id) {
  config.clear();
  char buffer[120];
  const bool pellet = id == Diagnostics::PelletQuantity;
  config["name"] = pellet ? "Quantit\u00e9 pellets" : "D\u00e9lai de standby";
  snprintf(buffer, sizeof(buffer), "322707E4_edilkamin_%s_setting", Diagnostics::fields[id].key);
  config["unique_id"] = buffer;
  topic(buffer, sizeof(buffer), id, "state");
  config["state_topic"] = buffer;
  topic(buffer, sizeof(buffer), id, "set");
  config["command_topic"] = buffer;
  config["optimistic"] = false;
  config["retain"] = false;
  config["device"]["identifiers"].to<JsonArray>().add("322707E4");
  config["entity_category"] = "config";
  if (pellet) {
    config["icon"] = "mdi:grain";
    JsonArray options = config["options"].to<JsonArray>();
    options.add("Bas");
    options.add("Moyen");
    options.add("Haut");
  } else {
    config["icon"] = "mdi:timer-sand";
    config["device_class"] = "duration";
    config["unit_of_measurement"] = "s";
    config["min"] = 60;
    config["max"] = 1800;
    config["step"] = 30;
    config["mode"] = "box";
  }
  topic(buffer, sizeof(buffer), id, "availability");
  availability(config, buffer);
}
inline void settingConfigTopic(char *out, size_t size, unsigned id) {
  snprintf(out, size, "homeassistant/%s/edilkamin_322707E4_%s/config",
           id == Diagnostics::PelletQuantity ? "select" : "number", Diagnostics::fields[id].key);
}
inline bool isSetting(unsigned id) {
  return id == Diagnostics::PelletQuantity || id == Diagnostics::StandbyDelay;
}
inline void techTopic(char *out, size_t size, unsigned id, const char *suffix) {
  snprintf(out, size, "edilkamin/322707E4/%s/%s", TechMenu::params[id].key, suffix);
}
inline void techConfigTopic(char *out, size_t size, unsigned id) {
  snprintf(out, size, "homeassistant/sensor/edilkamin_322707E4_%s/config",
           TechMenu::params[id].key);
}
// Technical menu values: read-only diagnostics, no statistics (constants).
inline void tech(JsonDocument &config, unsigned id) {
  config.clear();
  const TechMenu::Param &p = TechMenu::params[id];
  char buffer[120];
  config["name"] = p.name;
  snprintf(buffer, sizeof(buffer), "322707E4_edilkamin_%s", p.key);
  config["unique_id"] = buffer;
  techTopic(buffer, sizeof(buffer), id, "state");
  config["state_topic"] = buffer;
  device(config);
  if (p.unit) {
    config["unit_of_measurement"] = p.unit;
    if (!strcmp(p.unit, "\u00b0C")) config["device_class"] = "temperature";
    else if (!strcmp(p.unit, "s") || !strcmp(p.unit, "min")) config["device_class"] = "duration";
    else if (!strcmp(p.unit, "rpm")) config["icon"] = "mdi:fan";
    else config["icon"] = "mdi:percent";
  } else {
    config["icon"] = "mdi:gauge";
  }
  techTopic(buffer, sizeof(buffer), id, "availability");
  availability(config, buffer);
}
inline void button(JsonDocument &config, bool alarm) {
  config.clear();
  config["name"] = alarm ? "Alarm quittieren" : "Diagnose aktualisieren";
  config["unique_id"] = alarm ? "322707E4_edilkamin_alarm_reset" : "322707E4_edilkamin_diagnostics_refresh";
  config["command_topic"] = alarm ? "edilkamin/322707E4/alarm_reset/set" : "edilkamin/322707E4/diagnostics/refresh";
  config["payload_press"] = "PRESS";
  config["retain"] = false;
  config["icon"] = alarm ? "mdi:alarm-check" : "mdi:refresh";
  device(config);
  availability(config, alarm ? "edilkamin/322707E4/alarm_reset/availability" : nullptr);
}
inline void result(JsonDocument &config) {
  config.clear();
  config["name"] = "Alarm acknowledgment status";
  config["unique_id"] = "322707E4_edilkamin_alarm_reset_result";
  config["state_topic"] = "edilkamin/322707E4/alarm_reset_result/state";
  device(config);
  availability(config);
}
} // namespace DiagnosticDiscovery
