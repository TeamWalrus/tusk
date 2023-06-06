import React, { useState, useEffect, useRef } from "react";
import toast, { Toaster } from "react-hot-toast";
import formatBytes from "../components/FormatBytes";

export default function Settings() {
  const [openTab, setOpenTab] = React.useState(1);
  const [littlefsinfo, setLittleFSinfo] = useState([]);
  const [sdcardinfo, setSDCardInfo] = useState([]);
  const [wificonfig, setWiFiConfig] = useState([]);
  const [error, setError] = useState("");
  const form = useRef(null);

  const getLittleFSInfo = async () => {
    fetch("/api/littlefsinfo")
      .then((response) => response.json())
      .then((littlefsinfo) => {
        setLittleFSinfo(littlefsinfo);
      })
      .catch((error) => {
        setError(error);
      });
  };

  const getSDCardInfo = async () => {
    fetch("/api/sdcardinfo")
      .then((response) => response.json())
      .then((sdcardinfo) => {
        setSDCardInfo(sdcardinfo);
      })
      .catch((error) => {
        setError(error);
      });
  };

  const getWiFiConfig = async () => {
    fetch("/api/wificonfig")
      .then((response) => response.json())
      .then((currentwificonfig) => {
        setWiFiConfig(currentwificonfig);
        console.log(currentwificonfig);
      })
      .catch((error) => {
        setError(error);
      });
  };

  const submitWiFiUpdate = (e) => {
    e.preventDefault();
    fetch("/api/wificonfig/update", {
      method: "POST",
      body: new URLSearchParams(new FormData(form.current)),
      headers: {
        "Content-type": "application/x-www-form-urlencoded",
      },
    })
      // Getting 302 instead of 200 with response...
      .then((response) => response.text())
      .then((message) => {
        console.log(message);
        toast.success(message);
      })
      .catch((error) => {
        setError(error);
        toast.error(error);
      });
  };

  const deleteCardData = async () => {
    fetch("/api/delete/carddata")
      .then((response) => response.text())
      .then((message) => {
        console.log(message);
        toast.success(message);
      })
      .catch((error) => {
        setError(error);
        toast.error(error);
      });
  };

  useEffect(() => {
    getLittleFSInfo();
    getSDCardInfo();
    getWiFiConfig();
  }, []);

  const prettylittlefsinfototal = formatBytes(littlefsinfo.totalBytes);
  const prettylittlefsinfoused = formatBytes(littlefsinfo.usedBytes);
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

  const prettysdcardinfototal = formatBytes(sdcardinfo.totalBytes);
  const prettysdcardinfoused = formatBytes(sdcardinfo.usedBytes);
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

  const renderError = (
    <div className="flex justify-center items-center pt-6">
      <div className="text-center">
        <div className="alert alert-error shadow-lg">
          <div>
            <svg
              xmlns="http://www.w3.org/2000/svg"
              className="stroke-current flex-shrink-0 h-6 w-6"
              fill="none"
              viewBox="0 0 24 24"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                strokeWidth="2"
                d="M10 14l2-2m0 0l2-2m-2 2l-2-2m2 2l2 2m7-2a9 9 0 11-18 0 9 9 0 0118 0z"
              />
            </svg>
            <span>{error}</span>
          </div>
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
                "tab tab-lifted " + (openTab === 1 ? "tab-active" : "")
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
                "tab tab-lifted " + (openTab === 2 ? "tab-active" : "")
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
                "tab tab-lifted " + (openTab === 3 ? "tab-active" : "")
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
              className={openTab === 1 ? "block" : "hidden"}
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
              className={openTab === 2 ? "block" : "hidden"}
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
              <Toaster />
            </div>
            <div
              id="tab_littlefsinfo"
              className={openTab === 3 ? "block" : "hidden"}
            >
              {littlefsinfo && renderLittleFSInfo}
            </div>
            {error && renderError}
          </div>
        </article>
      </div>
    </div>
  );
}
