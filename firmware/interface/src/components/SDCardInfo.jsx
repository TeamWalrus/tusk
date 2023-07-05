import React, { useState, useEffect } from "react";
import formatBytes from "./FormatBytes";
import { postApiRequest, fetchApiRequest } from "../helpers/api";

export default function SDCardInfo({
  currentTab,
  showToastMessage,
  setErrorMessage,
}) {
  const opentab = currentTab;
  const [sdcardinfo, setSDCardInfo] = useState([]);

  const getSDCardInfo = async () => {
    try {
      const response = await fetchApiRequest("/api/device/sdcardinfo");
      setSDCardInfo(response);
    } catch (error) {
      setErrorMessage("An error occurred while fetching SD Card info.");
      console.error(error);
    }
  };

  const deleteCardData = async () => {
    try {
      const message = await postApiRequest("/api/carddata");
      showToastMessage(message);
    } catch (error) {
      setErrorMessage("An error occurred while deleting card data.");
      console.error(error);
    }
  };

  useEffect(() => {
    getSDCardInfo();
  }, []);

  const { totalBytes: sdcardTotalBytes, usedBytes: sdcardUsedBytes } =
    sdcardinfo;
  const prettysdcardinfototal = formatBytes(sdcardTotalBytes);
  const prettysdcardinfoused = formatBytes(sdcardUsedBytes);

  return (
    <div
      id="tab_sdcardinfo"
      className={opentab === "sd_card_info" ? "block" : "hidden"}
    >
      <div className="container">
        <div className="stats shadow">
          <div className="stat place-items-center">
            <div className="stat-title font-semibold text-info">Total</div>
            <div className="stat-value">{prettysdcardinfototal}</div>
          </div>
          <div className="stat place-items-center">
            <div className="stat-title font-semibold text-warning">Used</div>
            <div className="stat-value">{prettysdcardinfoused}</div>
          </div>
        </div>
      </div>

      <div className="container pt-6">
        <button
          className="btn-error btn"
          onClick={() => window.confirm_delete_modal.showModal()}
        >
          <svg
            xmlns="http://www.w3.org/2000/svg"
            fill="none"
            viewBox="0 0 24 24"
            strokeWidth={1.5}
            stroke="currentColor"
            className="h-6 w-6 pr-px"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              d="M12 9v3.75m-9.303 3.376c-.866 1.5.217 3.374 1.948 3.374h14.71c1.73 0 2.813-1.874 1.948-3.374L13.949 3.378c-.866-1.5-3.032-1.5-3.898 0L2.697 16.126zM12 15.75h.007v.008H12v-.008z"
            />
          </svg>
          Delete All Card Data
        </button>
        <dialog id="confirm_delete_modal" className="modal">
          <form method="dialog" className="modal-box">
            <h3 className="text-lg font-bold">
              Delete All Card Data
            </h3>
            <p className="py-4">
              All card data will be removed from the SD card!<br></br> This
              action cannot be undone.
            </p>
            <div className="modal-action flex justify-evenly">
              <button className="btn-info btn">Cancel</button>
              <button
                className="btn-error btn"
                onClick={() => {
                  deleteCardData();
                }}
              >
                Confirm
              </button>
            </div>
          </form>
        </dialog>
      </div>
    </div>
  );
}
