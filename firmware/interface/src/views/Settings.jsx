import React, { useState, useEffect, useRef } from "react";
import toast, { Toaster } from "react-hot-toast";
import formatBytes from "../components/FormatBytes";
import ErrorAlert from "../components/ErrorAlert";

export default function Settings() {
  const [opentab, setOpenTab] = useState(1);
  const [littlefsinfo, setLittleFSinfo] = useState([]);
  const [sdcardinfo, setSDCardInfo] = useState([]);
  const [wificonfig, setWiFiConfig] = useState([]);
  const [error, setError] = useState("");
  const form = useRef(null);

  const fetchApiRequest = async (url) => {
    try {
      const response = await fetch(url);
      if (!response.ok) {
        setError("Network error - Check console logs for additional information.");
        console.error(response.status);
      }
      return await response.json();
    } catch (error) {
      setError(
      "API error - Check console logs for additional information."
    );
    console.error(error);
    }
  };

  const getLittleFSInfo = async () => {
    try {
        const littlefsinfo = await fetchApiRequest("/api/littlefsinfo");
        setLittleFSinfo(littlefsinfo);
      } catch (error) {
        // setError(error) already handled in the fetchApiRequest
    }
  };

  const getSDCardInfo = async () => {
    try {
        const sdcardinfo = await fetchApiRequest("/api/sdcardinfo");
        setSDCardInfo(sdcardinfo);
      } catch (error) {
        // setError(error) already handled in the fetchApiRequest
    }
  };

  const getWiFiConfig = async () => {
    try {
        const currentwificonfig = await fetchApiRequest("/api/wificonfig");
        setWiFiConfig(currentwificonfig);
      } catch (error) {
        // setError(error) already handled in the fetchApiRequest
    }
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
        setError("Network error - Check console logs for additional information.");
        console.error(response.status);
      }

      return response.text();
    } catch (error) {
      setError(
      "API Error - Check console logs for additional information."
    );
    console.error(error);
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
        toast.success(message);
        postApiRequest("/api/device/reboot");
      })
      .catch((error) => {
        setError("API Error - Check console logs for additional information.");
        console.error(error);
      });
  };

  const deleteCardData = async () => {
    try {
      const message = await postApiRequest("/api/delete/carddata");
      toast.success(message);
    } catch (error) {
      // setError(error) already handled in the postApiRequest
    }
  };

  useEffect(() => {
    getLittleFSInfo();
    getSDCardInfo();
    getWiFiConfig();
  }, []);

  const { totalBytes: littlefsTotalBytes, usedBytes: littlefsUsedBytes } = littlefsinfo;
  const prettylittlefsinfototal = formatBytes(littlefsTotalBytes);
  const prettylittlefsinfoused = formatBytes(littlefsUsedBytes);

  const renderLittleFSInfo = (
    <div className="container">
      <div className="stats shadow">
        <div className="stat place-items-center ">
          <div className="stat-title text-info font-semibold">Total</div>
          <div className="stat-value">{prettylittlefsinfototal}</div>
        </div>
        <div className="stat place-items-center">
          <div className="stat-title text-warning font-semibold">Used</div>
          <div className="stat-value">{prettylittlefsinfoused}</div>
        </div>
      </div>
    </div>
  );
  
  const { totalBytes: sdcardTotalBytes, usedBytes: sdcardUsedBytes } = sdcardinfo;
  const prettysdcardinfototal = formatBytes(sdcardTotalBytes);
  const prettysdcardinfoused = formatBytes(sdcardUsedBytes);

  const renderSDCardInfo = (
    <div className="container">
      <div className="stats shadow">
        <div className="stat place-items-center ">
          <div className="stat-title text-info font-semibold">Total</div>
          <div className="stat-value">{prettysdcardinfototal}</div>
        </div>
        <div className="stat place-items-center">
          <div className="stat-title text-warning font-semibold">Used</div>
          <div className="stat-value">{prettysdcardinfoused}</div>
        </div>
      </div>
    </div>
  );

  return (
    <div className="flex justify-center items-center m-6">
      <div className="text-center">
        <article className="prose lg:prose-xl">
          <div className="tabs">
            <a
              className={
                "tab tab-lifted " + (opentab === 1 ? "tab-active" : "")
              }
              onClick={(e) => {
                e.preventDefault();
                setOpenTab(1);
              }}
            >
              WiFi Config
            </a>
            <a
              className={
                "tab tab-lifted " + (opentab === 2 ? "tab-active" : "")
              }
              onClick={(e) => {
                e.preventDefault();
                setOpenTab(2);
              }}
            >
              SD Card Info
            </a>
            <a
              className={
                "tab tab-lifted " + (opentab === 3 ? "tab-active" : "")
              }
              onClick={(e) => {
                e.preventDefault();
                setOpenTab(3);
              }}
            >
              LittleFS Info
            </a>
          </div>
          <div className="tabscontent pt-6">
            <div
              id="tab_wificonfig"
              className={opentab === 1 ? "block" : "hidden"}
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
                    className="input input-bordered input-primary w-full max-w-xs"
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
                    className="input input-bordered input-primary w-full max-w-xs"
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
                    className="input input-bordered input-primary w-full max-w-xs"
                  />
                  <label className="label cursor-pointer pt-4 pb-4">
                    <span className="label-text">Hide SSID</span>
                    <input
                      id="hidessid"
                      name="hidessid"
                      type="checkbox"
                      className="toggle toggle-success"
                      {...(wificonfig.hidessid === "0" ? "" : "defaultChecked")}
                    />
                  </label>
                  <button className="btn btn-success" value="submit">
                    Save & Reboot
                  </button>
                </div>
              </form>
            </div>
            <div
              id="tab_sdcardinfo"
              className={opentab === 2 ? "block" : "hidden"}
            >
              {sdcardinfo && renderSDCardInfo}
              <div className="container pt-6">
                <label htmlFor="confirm-delete-modal" className="btn btn-error">
                  Delete All Cards
                </label>
                <input
                  type="checkbox"
                  id="confirm-delete-modal"
                  className="modal-toggle checkbox-error"
                />
                <div className="modal">
                  <div className="modal-box">
                    <h3 className="font-bold text-lg">Delete All Card Data</h3>
                    <p className="py-4">
                      All card data will be removed from the SD card!<br></br>{" "}
                      This action cannot be undone.
                    </p>
                    <div className="modal-action flex justify-evenly">
                      <label
                        htmlFor="confirm-delete-modal"
                        className="btn btn-info"
                      >
                        Cancel
                      </label>
                      <label
                        htmlFor="confirm-delete-modal"
                        className="btn btn-error"
                        onClick={() => {
                          deleteCardData();
                        }}
                      >
                        Delete
                      </label>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            <div
              id="tab_littlefsinfo"
              className={opentab === 3 ? "block" : "hidden"}
            >
              {littlefsinfo && renderLittleFSInfo}
            </div>
            {error && <ErrorAlert message={error} />}
          </div>
        </article>
      </div>
    </div>
  );
}
