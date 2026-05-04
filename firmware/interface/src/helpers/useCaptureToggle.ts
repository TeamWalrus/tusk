import { useCallback } from "react";
import { postFormApiRequest } from "./api";
import { useDeviceSettings } from "../components/DeviceSettingsProvider";
import { useUI } from "../components/UIProvider";

export function useCaptureToggle() {
  const { deviceSettings, setDeviceSettings } = useDeviceSettings();
  const { setErrorMessage } = useUI();

  const toggle = useCallback(async () => {
    const next = !deviceSettings.capturing;
    try {
      await postFormApiRequest("/api/device/settings/general", {
        capturing: next,
      });
      setDeviceSettings((prev) => ({ ...prev, capturing: next }));
    } catch (error) {
      setErrorMessage("Failed to toggle capture state.");
      console.error(error);
    }
  }, [deviceSettings.capturing, setDeviceSettings, setErrorMessage]);

  return { capturing: deviceSettings.capturing, toggle };
}
