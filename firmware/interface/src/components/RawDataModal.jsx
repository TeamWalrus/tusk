export default function RawDataModal({ index, raw }) {
  return (
    <div>
      <label
        htmlFor={"rawdata_modal_" + index}
        className="btn-info btn-outline btn-sm btn"
      >
        Show
      </label>
      <input
        type="checkbox"
        id={"rawdata_modal_" + index}
        className="modal-toggle"
      />
      <div className="modal">
        <div className="modal-box">
          <h3 className="text-lg font-bold">Raw Data</h3>
          <p className="py-4 font-mono">{raw}</p>
        </div>
        <label className="modal-backdrop" htmlFor={"rawdata_modal_" + index}>
          Close
        </label>
      </div>
    </div>
  );
}
