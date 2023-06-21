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
            className="input input-bordered input-primary w-full max-w-xs"
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
            className="input input-bordered input-primary w-full max-w-xs"
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
            className="input input-bordered input-primary w-full max-w-xs"
          />
          <label className="label cursor-pointer pt-4 pb-4">
            <span className="label-text">Hide SSID</span>
            <input
              id="hidessid"
              name="hidessid"
              type="checkbox"
              className="toggle toggle-success"
              {...(config.hidessid === "0" ? "" : "defaultChecked")}
            />
          </label>
          <button className="btn btn-success" value="submit">
            Save & Reboot
          </button>
        </div>
      </form>
    </div>
  );
}
