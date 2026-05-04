import { useRef } from "react";
import { postApiRequest } from "../helpers/api";
import { useUI } from "./UIProvider";
import HoldToConfirm from "./HoldToConfirm";

export default function CardDataActions() {
  const { showToastMessage, setErrorMessage } = useUI();
  const wipeModal = useRef<HTMLDialogElement>(null);

  const handleWipe = async () => {
    try {
      const message = await postApiRequest("/api/carddata");
      wipeModal.current?.close();
      showToastMessage(message);
    } catch (error) {
      setErrorMessage("Failed to wipe card data.");
      console.error(error);
    }
  };

  return (
    <div className="card card-border bg-base-200">
      <div className="card-body">
        <h3 className="text-sm font-semibold uppercase tracking-wide opacity-70">
          Danger Zone
        </h3>
        <p className="text-sm opacity-80">
          Removes every captured card from the SD card. This cannot be undone.
        </p>
        <div className="card-actions">
          <button
            type="button"
            className="btn btn-error w-full gap-2"
            onClick={() => wipeModal.current?.showModal()}
          >
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
                d="M14.74 9l-.346 9m-4.788 0L9.26 9M19.228 5.79c.342.052.682.107 1.022.166m-1.022-.166L18.16 19.673a2.25 2.25 0 01-2.244 2.077H8.084a2.25 2.25 0 01-2.244-2.077L4.772 5.79m14.456 0a48.108 48.108 0 00-3.478-.397m-12 .562c.34-.059.68-.114 1.022-.165m0 0a48.11 48.11 0 013.478-.397m7.5 0v-.916c0-1.18-.91-2.164-2.09-2.201a51.964 51.964 0 00-3.32 0c-1.18.037-2.09 1.022-2.09 2.201v.916"
              />
            </svg>
            Wipe All Card Data
          </button>
        </div>

        <dialog ref={wipeModal} className="modal">
          <div className="modal-box">
            <h3 className="text-lg font-bold">Wipe all card data</h3>
            <p className="py-3 text-sm opacity-80">
              All captured cards will be permanently removed from the SD card.
              Hold the red button to confirm.
            </p>
            <HoldToConfirm
              label="Hold to wipe"
              holdingLabel="Wiping…"
              onConfirm={handleWipe}
            />
            <div className="modal-action">
              <form method="dialog">
                <button className="btn">Cancel</button>
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
