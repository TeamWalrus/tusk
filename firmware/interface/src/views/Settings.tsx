import ErrorAlert from "../components/ErrorAlert";
import StorageInfo from "../components/StorageInfo";
import WiFiConfig from "../components/WiFiConfig";
import ExportSection from "../components/ExportSection";
import DebugModeSection from "../components/DebugModeSection";
import CardDataActions from "../components/SDCardInfo";
import { useDeviceSettings } from "../components/DeviceSettingsProvider";
import { useUI } from "../components/UIProvider";

export default function Settings() {
  const { deviceSettings } = useDeviceSettings();
  const { errorMessage } = useUI();

  return (
    <div className="container mx-auto flex flex-col gap-4 px-3 py-4 sm:max-w-2xl">
      <div className="flex items-baseline justify-between">
        <h2 className="text-xl font-semibold">Settings</h2>
        <span className="text-sm opacity-60">
          firmware v{deviceSettings.version || "—"}
        </span>
      </div>

      <div className="grid grid-cols-1 gap-3 sm:grid-cols-2">
        <StorageInfo label="LittleFS" endpoint="/api/device/littlefsinfo" />
        <StorageInfo label="SD Card" endpoint="/api/device/sdcardinfo" />
      </div>

      <WiFiConfig />
      <DebugModeSection />
      <ExportSection />
      <CardDataActions />

      {errorMessage && <ErrorAlert message={errorMessage} />}
    </div>
  );
}
