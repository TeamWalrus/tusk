import React, { useState, useEffect, useRef } from "react";
import { postApiRequest, fetchApiRequest } from "../helpers/api";

export default function WiFiConfig({
  currentTab,
  showToastMessage,
  setErrorMessage,
}) {
  const wifi_config_tab = 2;
  const opentab = currentTab;
  const [wificonfig, setWiFiConfig] = useState([]);
  const form = useRef(null);

  const getWiFiConfig = async () => {
    try {
      const response = await fetchApiRequest("/api/device/wificonfig");
      setWiFiConfig(response);
    } catch (error) {
      setErrorMessage("An error occurred while fetching the WiFi config.");
      console.error(error);
    }
  };

  const submitWiFiUpdate = async (event) => {
    event.preventDefault();
    fetch("/api/device/wificonfig", {
      method: "POST",
      body: new URLSearchParams(new FormData(form.current)),
      headers: {
        "Content-type": "application/x-www-form-urlencoded",
      },
    })
      .then((response) => response.text())
      .then((message) => {
        showToastMessage(message);
        postApiRequest("/api/device/reboot");
      })
      .catch((error) => {
        setErrorMessage(
          "API Error - Check console logs for additional information."
        );
        console.error(error);
      });
  };

  useEffect(() => {
    getWiFiConfig();
  }, []);

  return (
    <div
      id="tab_wificonfig"
      className={opentab === wifi_config_tab ? "block" : "hidden"}
    >
      <form ref={form} onSubmit={submitWiFiUpdate}>
        <div className="form-control w-full max-w-xs">
          <label className="label">
            <span className="label-text">Name (SSID)</span>
          </label>
          <input
            id="ssid"
            name="ssid"
            type="text"
            placeholder={wificonfig.ssid}
            defaultValue={wificonfig.ssid}
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
            defaultValue={wificonfig.password}
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
            placeholder={wificonfig.channel}
            defaultValue={wificonfig.channel}
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
              defaultChecked={Boolean(wificonfig.hidessid)}
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
