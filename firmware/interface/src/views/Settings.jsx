import React, { useState } from "react";
import ErrorAlert from "../components/ErrorAlert";
import SDCardInfo from "../components/SDCardInfo";
import WiFiConfig from "../components/WiFiConfig";
import GeneralSettings from "../components/GeneralSettings";
import SettingsTabs from "../components/SettingsTabs";
import { DeviceSettingsProvider } from "../components/DeviceSettingsProvider";

export default function Settings() {
  const [currentTab, setCurrentTab] = useState(1);
  const [errorMessage, setErrorMessage] = useState("");
  const [toastMessage, setToastMessage] = useState("");
  const [toastTimeout, setToastTimeout] = useState(null);

  const showToastMessage = (message, duration = 4000) => {
    setToastMessage(message);
    clearTimeout(toastTimeout);
    const timeoutId = setTimeout(() => {
      setToastMessage("");
    }, duration);
    setToastTimeout(timeoutId);
  };

  return (
    <div className="flex items-center justify-center p-2">
      <div className="text-center">
        <div className="prose sm:prose-xl">
          {toastMessage && (
            <div className="toast-center toast toast-top">
              <div className="alert alert-success">
                <span>{toastMessage}</span>
              </div>
            </div>
          )}
          <div className="tabs justify-center">
            {
              <SettingsTabs
                currentTab={currentTab}
                setCurrentTab={setCurrentTab}
              />
            }
          </div>
          <div className="tabscontent pt-6">
            <GeneralSettings
              currentTab={currentTab}
              showToastMessage={showToastMessage}
              setErrorMessage={setErrorMessage}
            />
            <WiFiConfig
              currentTab={currentTab}
              showToastMessage={showToastMessage}
              setErrorMessage={setErrorMessage}
            />
            <SDCardInfo
              currentTab={currentTab}
              showToastMessage={showToastMessage}
              setErrorMessage={setErrorMessage}
            />
            {errorMessage && <ErrorAlert message={errorMessage} />}
          </div>
        </div>
      </div>
    </div>
  );
}
