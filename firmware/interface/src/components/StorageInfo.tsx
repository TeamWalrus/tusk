import { useEffect, useState } from "react";
import { fetchApiRequest } from "../helpers/api";
import { useUI } from "./UIProvider";
import formatBytes from "./FormatBytes";
import type { FilesystemInfo } from "../types/api";

interface Props {
  label: string;
  endpoint: "/api/device/littlefsinfo" | "/api/device/sdcardinfo";
}

export default function StorageInfo({ label, endpoint }: Props) {
  const { setErrorMessage } = useUI();
  const [info, setInfo] = useState<FilesystemInfo | null>(null);

  useEffect(() => {
    fetchApiRequest<FilesystemInfo>(endpoint)
      .then(setInfo)
      .catch((error) => {
        setErrorMessage(`Failed to fetch ${label} info.`);
        console.error(error);
      });
  }, [endpoint, label, setErrorMessage]);

  const total = info?.totalBytes ?? 0;
  const used = info?.usedBytes ?? 0;
  const pct = total > 0 ? Math.min(100, (used / total) * 100) : 0;

  return (
    <div className="card card-border bg-base-200">
      <div className="card-body p-4">
        <div className="flex items-baseline justify-between">
          <span className="text-sm font-semibold uppercase tracking-wide opacity-70">
            {label}
          </span>
          <span className="font-mono text-xs opacity-70">
            {formatBytes(used)} / {formatBytes(total)}
          </span>
        </div>
        <progress
          className="progress progress-info mt-2 w-full"
          value={pct}
          max={100}
        />
      </div>
    </div>
  );
}
