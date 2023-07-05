export default function RawDataModal({ index, raw }) {
  const id = "rawdata_modal_" + index;
  const openModal = () => {
    const modal = document.getElementById(id);
    if (modal) modal.showModal();
  };

  return (
    <div>
      <button className="btn-info btn-outline btn-sm btn" onClick={openModal}>
        show
      </button>
      <dialog id={id} className="modal">
        <form method="dialog" className="modal-box">
          <h3 className="text-lg font-bold">Raw Data</h3>
          <p className="py-4 font-mono break-words overflow-auto">{raw}</p>
        </form>
        <form method="dialog" className="modal-backdrop">
          <button>close</button>
        </form>
      </dialog>
    </div>
  );
}
