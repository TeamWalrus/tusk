import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useState,
  type Dispatch,
  type ReactNode,
  type SetStateAction,
} from "react";
import { fetchApiRequest } from "../helpers/api";
import type { DeviceSettings } from "../types/api";

interface ContextValue {
  deviceSettings: DeviceSettings;
  setDeviceSettings: Dispatch<SetStateAction<DeviceSettings>>;
  refreshDeviceSettings: () => Promise<void>;
}

const DeviceSettingsContext = createContext<ContextValue | null>(null);

const DEFAULT_SETTINGS: DeviceSettings = {
  capturing: true,
  debug_mode: false,
  version: "",
};

export function DeviceSettingsProvider({ children }: { children: ReactNode }) {
  const [deviceSettings, setDeviceSettings] =
    useState<DeviceSettings>(DEFAULT_SETTINGS);

  const refreshDeviceSettings = useCallback(async () => {
    try {
      const response = await fetchApiRequest<DeviceSettings>(
        "/api/device/settings/general"
      );
      setDeviceSettings((prev) => ({ ...prev, ...response }));
    } catch (error) {
      console.error(
        "An error occurred while fetching device's general settings.",
        error
      );
    }
  }, []);

  useEffect(() => {
    refreshDeviceSettings();
  }, [refreshDeviceSettings]);

  const value = useMemo<ContextValue>(
    () => ({ deviceSettings, setDeviceSettings, refreshDeviceSettings }),
    [deviceSettings, refreshDeviceSettings]
  );

  return (
    <DeviceSettingsContext.Provider value={value}>
      {children}
    </DeviceSettingsContext.Provider>
  );
}

export function useDeviceSettings(): ContextValue {
  const ctx = useContext(DeviceSettingsContext);
  if (!ctx) {
    throw new Error(
      "useDeviceSettings must be used inside <DeviceSettingsProvider>"
    );
  }
  return ctx;
}
