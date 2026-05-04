import { useEffect, useRef, useState } from "react";
import {
  fetchApiRequest,
  postApiRequest,
  postFormApiRequest,
} from "../helpers/api";
import { useUI } from "./UIProvider";
import type { WiFiConfig as WiFiConfigType } from "../types/api";

const EMPTY_CONFIG: WiFiConfigType = {
  ssid: "",
  password: "",
  channel: "1",
  hidessid: "0",
};

// 2.4 GHz channels valid across most regulatory domains. 1 / 6 / 11 are the
// non-overlapping favorites and are flagged in the labels.
const CHANNELS = [
  { value: "1", label: "1 (non-overlapping)" },
  { value: "2", label: "2" },
  { value: "3", label: "3" },
  { value: "4", label: "4" },
  { value: "5", label: "5" },
  { value: "6", label: "6 (non-overlapping)" },
  { value: "7", label: "7" },
  { value: "8", label: "8" },
  { value: "9", label: "9" },
  { value: "10", label: "10" },
  { value: "11", label: "11 (non-overlapping)" },
  { value: "12", label: "12" },
  { value: "13", label: "13" },
];

export default function WiFiConfig() {
  const { showToastMessage, setErrorMessage } = useUI();
  const [wificonfig, setWiFiConfig] = useState<WiFiConfigType>(EMPTY_CONFIG);
  const [showPassword, setShowPassword] = useState(false);
  const form = useRef<HTMLFormElement>(null);
  const confirmModal = useRef<HTMLDialogElement>(null);
  const pendingFormData = useRef<FormData | null>(null);

  useEffect(() => {
    fetchApiRequest<WiFiConfigType>("/api/device/wificonfig")
      .then((cfg) => setWiFiConfig({ ...cfg, channel: cfg.channel || "1" }))
      .catch((error) => {
        setErrorMessage("Failed to fetch WiFi config.");
        console.error(error);
      });
  }, [setErrorMessage]);

  const requestSave = (event: React.FormEvent<HTMLFormElement>) => {
    event.preventDefault();
    if (!form.current) return;
    pendingFormData.current = new FormData(form.current);
    confirmModal.current?.showModal();
  };

  const confirmSave = async () => {
    if (!pendingFormData.current) return;
    confirmModal.current?.close();
    try {
      const message = await postFormApiRequest(
        "/api/device/wificonfig",
        pendingFormData.current
      );
      showToastMessage(message);
      await postApiRequest("/api/device/reboot");
    } catch (error) {
      setErrorMessage("Failed to update WiFi config.");
      console.error(error);
    } finally {
      pendingFormData.current = null;
    }
  };

  return (
    <div className="card card-border bg-base-200">
      <div className="card-body">
        <h3 className="text-sm font-semibold uppercase tracking-wide opacity-70">
          Access Point
        </h3>

        <form ref={form} onSubmit={requestSave} className="flex flex-col gap-4">
          <div>
            <label
              htmlFor="ssid"
              className="mb-1 block text-sm font-medium"
            >
              Name (SSID)
            </label>
            <input
              id="ssid"
              name="ssid"
              type="text"
              defaultValue={wificonfig.ssid}
              required
              className="input w-full"
              key={`ssid-${wificonfig.ssid}`}
            />
          </div>

          <div>
            <label
              htmlFor="password"
              className="mb-1 block text-sm font-medium"
            >
              Password
            </label>
            <div className="join w-full">
              <input
                id="password"
                name="password"
                type={showPassword ? "text" : "password"}
                defaultValue={wificonfig.password}
                required
                minLength={8}
                className="input join-item w-full"
                key={`pw-${wificonfig.password}`}
              />
              <button
                type="button"
                className="btn join-item"
                onClick={() => setShowPassword((v) => !v)}
                aria-label={showPassword ? "Hide password" : "Show password"}
              >
                {showPassword ? (
                  <svg
                    xmlns="http://www.w3.org/2000/svg"
                    fill="none"
                    viewBox="0 0 24 24"
                    strokeWidth={1.8}
                    stroke="currentColor"
                    className="h-5 w-5"
                  >
                    <path
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      d="M3.98 8.223A10.477 10.477 0 001.934 12C3.226 16.338 7.244 19.5 12 19.5c.993 0 1.953-.138 2.863-.395M6.228 6.228A10.45 10.45 0 0112 4.5c4.756 0 8.773 3.162 10.065 7.498a10.522 10.522 0 01-4.293 5.774M6.228 6.228L3 3m3.228 3.228l3.65 3.65m7.894 7.894L21 21m-3.228-3.228l-3.65-3.65m0 0a3 3 0 10-4.243-4.243m4.243 4.243L9.88 9.88"
                    />
                  </svg>
                ) : (
                  <svg
                    xmlns="http://www.w3.org/2000/svg"
                    fill="none"
                    viewBox="0 0 24 24"
                    strokeWidth={1.8}
                    stroke="currentColor"
                    className="h-5 w-5"
                  >
                    <path
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      d="M2.036 12.322a1.012 1.012 0 010-.639C3.423 7.51 7.36 4.5 12 4.5c4.638 0 8.573 3.007 9.963 7.178.07.207.07.431 0 .639C20.577 16.49 16.64 19.5 12 19.5c-4.638 0-8.573-3.007-9.963-7.178z"
                    />
                    <path
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"
                    />
                  </svg>
                )}
              </button>
            </div>
          </div>

          <div>
            <label
              htmlFor="channel"
              className="mb-1 block text-sm font-medium"
            >
              Channel
            </label>
            <select
              id="channel"
              name="channel"
              defaultValue={wificonfig.channel}
              className="select w-full"
              key={`ch-${wificonfig.channel}`}
            >
              {CHANNELS.map((c) => (
                <option key={c.value} value={c.value}>
                  {c.label}
                </option>
              ))}
            </select>
          </div>

          <label className="flex cursor-pointer items-center justify-between">
            <span className="text-sm font-medium">Hide SSID</span>
            <input
              id="hidessid"
              name="hidessid"
              type="checkbox"
              className="toggle toggle-success"
              defaultChecked={wificonfig.hidessid === "1"}
              key={`hide-${wificonfig.hidessid}`}
            />
          </label>

          <button type="submit" className="btn btn-success">
            Save & Reboot
          </button>
        </form>

        <dialog ref={confirmModal} className="modal">
          <div className="modal-box">
            <h3 className="text-lg font-bold">Save and reboot?</h3>
            <p className="py-3 text-sm opacity-80">
              The device will reboot to apply the new WiFi config. Your current
              connection to the access point will drop and you'll need to
              reconnect with the new credentials.
            </p>
            <div className="modal-action">
              <form method="dialog" className="flex gap-2">
                <button className="btn">Cancel</button>
                <button
                  type="button"
                  className="btn btn-warning"
                  onClick={confirmSave}
                >
                  Save & Reboot
                </button>
              </form>
            </div>
          </div>
          <form method="dialog" className="modal-backdrop">
            <button>close</button>
          </form>
        </dialog>
      </div>
    </div>
  );
}
