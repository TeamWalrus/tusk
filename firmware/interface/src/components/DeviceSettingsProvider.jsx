import React, { useState, useEffect, createContext } from "react";
import { fetchApiRequest } from "../helpers/api";

export const DeviceSettingsContext = createContext();

export const DeviceSettingsProvider = ({ children }) => {
  const [deviceSettings, setDeviceSettings] = useState({ capturing: true });

  const getDeviceSettings = async () => {
    try {
      const response = await fetchApiRequest("/api/device/settings/general");
      setDeviceSettings((prevSettings) => ({ ...prevSettings, ...response }));
    } catch (error) {
      console.error(
        "An error occurred while fetching device's general settings."
      );
    }
  };

  useEffect(() => {
    getDeviceSettings();
  }, []);

  return (
    <DeviceSettingsContext.Provider
      value={{ deviceSettings, setDeviceSettings, getDeviceSettings }}
    >
      {children}
    </DeviceSettingsContext.Provider>
  );
};
