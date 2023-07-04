export default function SettingsTabs({ currentTab, setCurrentTab }) {
  const tabs = {
    general_settings: 1,
    wifi_config: 2,
    sd_card_info: 3,
  };

  return (
    <div>
      <a
        className={`tab-lifted tab ${currentTab === tabs.general_settings ? "tab-active" : ""}`}
        onClick={(event) => {
          event.preventDefault();
          setCurrentTab(tabs.general_settings);
        }}
      >
        General
      </a>
      <a
        className={`tab-lifted tab ${currentTab === tabs.wifi_config ? "tab-active" : ""}`}
        onClick={(event) => {
          event.preventDefault();
          setCurrentTab(tabs.wifi_config);
        }}
      >
        WiFi Config
      </a>
      <a
        className={`tab-lifted tab ${currentTab === tabs.sd_card_info ? "tab-active" : ""}`}
        onClick={(event) => {
          event.preventDefault();
          setCurrentTab(tabs.sd_card_info);
        }}
      >
        SD Card
      </a>
    </div>
  );
}
