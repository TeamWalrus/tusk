import React, { useState, useEffect, useContext } from "react";
import formatBytes from "./FormatBytes";
import { fetchApiRequest } from "../helpers/api";
import { DeviceSettingsContext } from "./DeviceSettingsProvider";

export default function GeneralSettings({ tab, setError }) {
  const general_settings = 1;
  const opentab = tab;
  const { deviceSettings, setDeviceSettings, getDeviceSettings } = useContext(
    DeviceSettingsContext
  );
  const [littlefsinfo, setLittleFSinfo] = useState([]);

  const getLittleFSInfo = async () => {
    try {
      const response = await fetchApiRequest("/api/device/littlefsinfo");
      setLittleFSinfo(response);
    } catch (error) {
      setError("An error occurred while fetching LittleFS info.");
      console.error(error);
    }
  };

  const handleToggleCapturing = async (event) => {
    try {
      const newCapturingState = event.target.checked;
      const formData = new URLSearchParams();
      formData.append("capturing", newCapturingState);

      const response = await fetch("/api/device/settings/general", {
        method: "POST",
        headers: {
          "Content-Type": "application/x-www-form-urlencoded",
        },
        body: formData,
      });

      if (response.status !== 200) {
        throw new Error(
          `Network Error: ${response.status}, ${response.statusText}`
        );
      }

      setDeviceSettings((prevState) => ({
        ...prevState,
        capturing: newCapturingState,
      }));
    } catch (error) {
      setError("An error occurred while updating capturing setting:");
      console.error(error);
    }
  };

  useEffect(() => {
    getDeviceSettings();
    getLittleFSInfo();
  }, []);

  const { totalBytes: littlefsTotalBytes, usedBytes: littlefsUsedBytes } =
    littlefsinfo;
  const prettylittlefsinfototal = formatBytes(littlefsTotalBytes);
  const prettylittlefsinfoused = formatBytes(littlefsUsedBytes);

  return (
    <div
      id="tab_general_settings"
      className={`flex flex-col items-center ${
        opentab === general_settings ? "block" : "hidden"
      }`}
    >
      <div className="pb-2 text-center">
        <span className="label-text pr-2">Version:</span>
        <span className="text-sm">{deviceSettings.version}</span>
      </div>
      <div className="form-control pb-2">
        <label className="label cursor-pointer">
          <span className="label-text pr-2">Capture Data: </span>
          <input
            id="toggleCapturing"
            name="toggleCapturing"
            type="checkbox"
            className="toggle-success toggle px-4"
            checked={deviceSettings.capturing}
            onChange={handleToggleCapturing}
          />
        </label>
      </div>
      <div className="container">
        <h4>LittleFS Info:</h4>
        <div className="stats shadow">
          <div className="stat place-items-center">
            <div className="stat-title font-semibold text-info">Total</div>
            <div className="stat-value">{prettylittlefsinfototal}</div>
          </div>
          <div className="stat place-items-center">
            <div className="stat-title font-semibold text-warning">Used</div>
            <div className="stat-value">{prettylittlefsinfoused}</div>
          </div>
        </div>
      </div>
    </div>
  );
}
