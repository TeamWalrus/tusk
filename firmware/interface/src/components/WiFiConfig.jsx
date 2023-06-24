export default function WiFiConfig({ tab, config, form, onSubmit }) {
  const wifi_config_tab = 1;
  const opentab = tab;
  return (
    <div
      id="tab_wificonfig"
      className={opentab === wifi_config_tab ? "block" : "hidden"}
    >
      <form ref={form} onSubmit={onSubmit}>
        <div className="form-control w-full max-w-xs">
          <label className="label">
            <span className="label-text">Name (SSID)</span>
          </label>
          <input
            id="ssid"
            name="ssid"
            type="text"
            placeholder={config.ssid}
            defaultValue={config.ssid}
            required
            className="input-bordered input-primary input w-full max-w-xs"
          />
          <label className="label">
            <span className="label-text">Password</span>
          </label>
          <input
            id="password"
            name="password"
            type="password"
            placeholder="********"
            defaultValue={config.password}
            required
            minLength={8}
            className="input-bordered input-primary input w-full max-w-xs"
          />
          <label className="label">
            <span className="label-text">Channel</span>
          </label>
          <input
            id="channel"
            name="channel"
            type="text"
            placeholder={config.channel}
            defaultValue={config.channel}
            required
            className="input-bordered input-primary input w-full max-w-xs"
          />
          <label className="label cursor-pointer pb-4 pt-4">
            <span className="label-text">Hide SSID</span>
            <input
              id="hidessid"
              name="hidessid"
              type="checkbox"
              className="toggle-success toggle"
              {...(config.hidessid === "0" ? "" : "defaultChecked")}
            />
          </label>
          <button className="btn-success btn" value="submit">
            Save & Reboot
          </button>
        </div>
      </form>
    </div>
  );
}
