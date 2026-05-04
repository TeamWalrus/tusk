import { useEffect, useRef, useState } from "react";

interface Props {
  label: string;
  holdingLabel?: string;
  onConfirm: () => void;
  durationMs?: number;
  className?: string;
}

export default function HoldToConfirm({
  label,
  holdingLabel = "Keep holding…",
  onConfirm,
  durationMs = 1500,
  className = "",
}: Props) {
  const [progress, setProgress] = useState(0);
  const startedAt = useRef<number | null>(null);
  const raf = useRef<number | null>(null);

  const reset = () => {
    startedAt.current = null;
    if (raf.current !== null) {
      cancelAnimationFrame(raf.current);
      raf.current = null;
    }
    setProgress(0);
  };

  useEffect(() => () => reset(), []);

  const start = () => {
    if (startedAt.current !== null) return;
    startedAt.current = performance.now();
    const tick = () => {
      if (startedAt.current === null) return;
      const elapsed = performance.now() - startedAt.current;
      const p = Math.min(elapsed / durationMs, 1);
      setProgress(p);
      if (p >= 1) {
        reset();
        onConfirm();
        return;
      }
      raf.current = requestAnimationFrame(tick);
    };
    raf.current = requestAnimationFrame(tick);
  };

  const cancel = () => reset();

  const holding = progress > 0;

  return (
    <button
      type="button"
      onPointerDown={start}
      onPointerUp={cancel}
      onPointerLeave={cancel}
      onPointerCancel={cancel}
      className={
        "btn btn-error relative w-full overflow-hidden select-none " + className
      }
    >
      <span
        className="absolute inset-y-0 left-0 bg-error-content/20"
        style={{ width: `${progress * 100}%`, transition: "width 30ms linear" }}
      />
      <span className="relative">{holding ? holdingLabel : label}</span>
    </button>
  );
}
