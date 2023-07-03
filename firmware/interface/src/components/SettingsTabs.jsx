export default function SettingsTabs({ tab, setOpenTab }) {
  const tabs = {
    general_settings: 1,
    wifi_config: 2,
    sd_card_info: 3,
  };
  const opentab = tab;
  return (
    <div>
      <a
        className={`tab-lifted tab ${opentab === tabs.general_settings ? "tab-active" : ""}`}
        onClick={(event) => {
          event.preventDefault();
          setOpenTab(tabs.general_settings);
        }}
      >
        General
      </a>
      <a
        className={`tab-lifted tab ${opentab === tabs.wifi_config ? "tab-active" : ""}`}
        onClick={(event) => {
          event.preventDefault();
          setOpenTab(tabs.wifi_config);
        }}
      >
        WiFi Config
      </a>
      <a
        className={`tab-lifted tab ${opentab === tabs.sd_card_info ? "tab-active" : ""}`}
        onClick={(event) => {
          event.preventDefault();
          setOpenTab(tabs.sd_card_info);
        }}
      >
        SD Card
      </a>
    </div>
  );
}
