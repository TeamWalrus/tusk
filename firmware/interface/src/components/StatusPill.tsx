import { useCaptureToggle } from "../helpers/useCaptureToggle";

export default function StatusPill() {
  const { capturing, toggle } = useCaptureToggle();

  return (
    <button
      onClick={toggle}
      className={
        "btn btn-sm gap-2 normal-case " +
        (capturing
          ? "btn-success text-success-content"
          : "btn-error text-error-content")
      }
      aria-label={
        capturing ? "Capture armed — tap to disarm" : "Capture off — tap to arm"
      }
      title={capturing ? "Tap to disarm" : "Tap to arm"}
    >
      <span
        className={
          "inline-block h-2 w-2 rounded-full " +
          (capturing ? "animate-pulse bg-base-100" : "bg-base-100/70")
        }
      />
      {capturing ? "ARMED" : "OFF"}
    </button>
  );
}
