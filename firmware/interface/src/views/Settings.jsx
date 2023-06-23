import React, { useState, useEffect, useRef } from "react";
import ErrorAlert from "../components/ErrorAlert";
import LittleFSInfo from "../components/LittleFSInfo";
import SDCardInfo from "../components/SDCardInfo";
import WiFiConfig from "../components/WiFiConfig";
import SettingsTabs from "../components/SettingsTabs";

export default function Settings() {
  const [opentab, setOpenTab] = useState(1);
  const [littlefsinfo, setLittleFSinfo] = useState([]);
  const [sdcardinfo, setSDCardInfo] = useState([]);
  const [wificonfig, setWiFiConfig] = useState([]);
  const [error, setError] = useState("");
  const [toastMessage, setToastMessage] = useState("");
  const [toastTimeout, setToastTimeout] = useState(null);
  const form = useRef(null);

  const clearError = () => setError("");

  const showToast = (message, duration = 4000) => {
    setToastMessage(message);
    clearTimeout(toastTimeout);
    const timeoutId = setTimeout(() => {
      setToastMessage("");
    }, duration);
    setToastTimeout(timeoutId);
  };

  const postApiRequest = async (url, data = {}) => {
    try {
      const response = await fetch(url, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(data),
      });

      if (!response.ok) {
        throw new Error(
          "Network error - Check console logs for additional information."
        );
      }
      clearError();
      return response.text();
    } catch (error) {
      console.error(
        "API Error - Check console logs for additional information.",
        error
      );
      throw error;
    }
  };

  const fetchApiRequest = async (url) => {
    try {
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(
          "Network error - Check console logs for additional information."
        );
      }
      clearError();
      return await response.json();
    } catch (error) {
      console.error(
        "API Error - Check console logs for additional information.",
        error
      );
      throw error;
    }
  };

  const getLittleFSInfo = async () => {
    try {
      const littlefsinfo = await fetchApiRequest("/api/littlefsinfo");
      setLittleFSinfo(littlefsinfo);
    } catch (error) {
      setError("An error occurred while fetching LittleFS info.");
    }
  };

  const getSDCardInfo = async () => {
    try {
      const sdcardinfo = await fetchApiRequest("/api/sdcardinfo");
      setSDCardInfo(sdcardinfo);
    } catch (error) {
      setError("An error occurred while fetching SD Card info.");
    }
  };

  const getWiFiConfig = async () => {
    try {
      const currentwificonfig = await fetchApiRequest("/api/wificonfig");
      setWiFiConfig(currentwificonfig);
    } catch (error) {
      setError("An error occurred while fetching the WiFi config.");
    }
  };

  const submitWiFiUpdate = async (e) => {
    e.preventDefault();
    fetch("/api/wificonfig/update", {
      method: "POST",
      body: new URLSearchParams(new FormData(form.current)),
      headers: {
        "Content-type": "application/x-www-form-urlencoded",
      },
    })
      .then((response) => response.text())
      .then((message) => {
        showToast(message);
        postApiRequest("/api/device/reboot");
      })
      .catch((error) => {
        console.error(
          "API Error - Check console logs for additional information.",
          error
        );
        throw error;
      });
  };

  const deleteCardData = async () => {
    try {
      const message = await postApiRequest("/api/delete/carddata");
      showToast(message);
    } catch (error) {
      setError("An error occurred while deleting card data.");
    }
  };

  useEffect(() => {
    getLittleFSInfo();
    getSDCardInfo();
    getWiFiConfig();
  }, []);

  return (
    <div className="flex justify-center items-center p-2">
      <div className="text-center">
        <div className="prose sm:prose-xl">
          {toastMessage && (
            <div className="toast toast-top toast-center">
              <div className="alert alert-success">
                <span>{toastMessage}</span>
              </div>
            </div>
          )}
          <div className="tabs justify-center">
            {<SettingsTabs tab={opentab} onSetOpenTab={setOpenTab} />}
          </div>
          <div className="tabscontent pt-6">
            {wificonfig && (
              <WiFiConfig
                tab={opentab}
                config={wificonfig}
                form={form}
                onSubmit={submitWiFiUpdate}
              />
            )}
            {sdcardinfo && (
              <SDCardInfo
                tab={opentab}
                data={sdcardinfo}
                onDelete={deleteCardData}
              />
            )}
            {littlefsinfo && <LittleFSInfo tab={opentab} data={littlefsinfo} />}
            {error && <ErrorAlert message={error} />}
          </div>
        </div>
      </div>
    </div>
  );
}
