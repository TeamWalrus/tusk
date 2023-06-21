export default function SettingsTabs({ tab, onSetOpenTab }) {
  const tabs = {
    wifi_config: 1,
    sd_card_info: 2,
    little_fs_info: 3,
  };
  const opentab = tab;
  return (
    <div>
      <a
        className={
          "tab tab-lifted " + (opentab === tabs.wifi_config ? "tab-active" : "")
        }
        onClick={(e) => {
          e.preventDefault();
          onSetOpenTab(tabs.wifi_config);
        }}
      >
        WiFi Config
      </a>
      <a
        className={
          "tab tab-lifted " +
          (opentab === tabs.sd_card_info ? "tab-active" : "")
        }
        onClick={(e) => {
          e.preventDefault();
          onSetOpenTab(tabs.sd_card_info);
        }}
      >
        SD Card Info
      </a>
      <a
        className={
          "tab tab-lifted " +
          (opentab === tabs.little_fs_info ? "tab-active" : "")
        }
        onClick={(e) => {
          e.preventDefault();
          onSetOpenTab(tabs.little_fs_info);
        }}
      >
        LittleFS Info
      </a>
    </div>
  );
}
