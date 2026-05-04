#include "config_store.h"
#include <Preferences.h>

namespace {
constexpr const char *NAMESPACE_WIFI = "tusk-wifi";
constexpr const char *NAMESPACE_GENERAL = "tusk-general";

// Defaults applied on first boot when NVS has no stored value.
constexpr const char *DEFAULT_SSID = "Tusk";
constexpr const char *DEFAULT_PASSWORD = "changeme";
constexpr uint8_t DEFAULT_CHANNEL = 1;
constexpr bool DEFAULT_HIDDEN = false;
constexpr bool DEFAULT_CAPTURING = true;
constexpr bool DEFAULT_DEBUG = false;
}  // namespace

namespace config_store {

void begin() {
  // Touch each namespace so it's created if missing. Preferences::begin
  // does not error when the namespace doesn't exist yet.
  Preferences prefs;
  prefs.begin(NAMESPACE_WIFI, false);
  prefs.end();
  prefs.begin(NAMESPACE_GENERAL, false);
  prefs.end();
}

WiFiSettings getWiFi() {
  Preferences prefs;
  prefs.begin(NAMESPACE_WIFI, true);
  WiFiSettings s{
      prefs.getString("ssid", DEFAULT_SSID),
      prefs.getString("pw", DEFAULT_PASSWORD),
      prefs.getUChar("channel", DEFAULT_CHANNEL),
      prefs.getBool("hidden", DEFAULT_HIDDEN),
  };
  prefs.end();
  return s;
}

void setWiFi(const WiFiSettings &settings) {
  Preferences prefs;
  prefs.begin(NAMESPACE_WIFI, false);
  prefs.putString("ssid", settings.ssid);
  prefs.putString("pw", settings.password);
  prefs.putUChar("channel", settings.channel);
  prefs.putBool("hidden", settings.hidden);
  prefs.end();
}

bool getCapturing() {
  Preferences prefs;
  prefs.begin(NAMESPACE_GENERAL, true);
  bool value = prefs.getBool("capturing", DEFAULT_CAPTURING);
  prefs.end();
  return value;
}

void setCapturing(bool capturing) {
  Preferences prefs;
  prefs.begin(NAMESPACE_GENERAL, false);
  prefs.putBool("capturing", capturing);
  prefs.end();
}

bool getDebugMode() {
  Preferences prefs;
  prefs.begin(NAMESPACE_GENERAL, true);
  bool value = prefs.getBool("debug", DEFAULT_DEBUG);
  prefs.end();
  return value;
}

void setDebugMode(bool debug) {
  Preferences prefs;
  prefs.begin(NAMESPACE_GENERAL, false);
  prefs.putBool("debug", debug);
  prefs.end();
}

}  // namespace config_store
