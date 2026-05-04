import { postFormApiRequest } from "../helpers/api";
import { useDeviceSettings } from "./DeviceSettingsProvider";
import { useUI } from "./UIProvider";

export default function DebugModeSection() {
  const { deviceSettings, setDeviceSettings } = useDeviceSettings();
  const { setErrorMessage } = useUI();

  const handleToggle = async (event: React.ChangeEvent<HTMLInputElement>) => {
    const next = event.target.checked;
    try {
      await postFormApiRequest("/api/device/settings/general", {
        debug_mode: next,
      });
      setDeviceSettings((prev) => ({ ...prev, debug_mode: next }));
    } catch (error) {
      setErrorMessage("Failed to toggle debug mode.");
      console.error(error);
    }
  };

  return (
    <div className="card card-border bg-base-200">
      <div className="card-body">
        <h3 className="text-sm font-semibold uppercase tracking-wide opacity-70">
          Debug
        </h3>
        <label className="flex cursor-pointer items-start justify-between gap-3">
          <div className="flex-1">
            <div className="font-medium">Debug mode</div>
            <p className="text-sm opacity-70">
              Bypass the duplicate-read filter so the same test card can be
              presented repeatedly to verify the reader pipeline. Every read is
              logged and broadcast — disable for normal operation.
            </p>
          </div>
          <input
            type="checkbox"
            className="toggle toggle-warning mt-1 shrink-0"
            checked={deviceSettings.debug_mode}
            onChange={handleToggle}
          />
        </label>
      </div>
    </div>
  );
}
