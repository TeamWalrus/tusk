import formatBytes from "./FormatBytes";

export default function SDCardInfo({ tab, data, onDelete }) {
  const sd_card_info_tab = 2;
  const opentab = tab;
  const { totalBytes: sdcardTotalBytes, usedBytes: sdcardUsedBytes } = data;
  const prettysdcardinfototal = formatBytes(sdcardTotalBytes);
  const prettysdcardinfoused = formatBytes(sdcardUsedBytes);

  return (
    <div
      id="tab_sdcardinfo"
      className={opentab === sd_card_info_tab ? "block" : "hidden"}
    >
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
              All card data will be removed from the SD card!<br></br> This
              action cannot be undone.
            </p>
            <div className="modal-action flex justify-evenly">
              <label htmlFor="confirm-delete-modal" className="btn btn-info">
                Cancel
              </label>
              <label
                htmlFor="confirm-delete-modal"
                className="btn btn-error"
                onClick={() => {
                  onDelete();
                }}
              >
                Delete
              </label>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
