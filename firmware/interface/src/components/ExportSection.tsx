import { useState } from "react";
import type { Card } from "../types/api";
import { downloadCsv } from "../helpers/exportCsv";
import { useUI } from "./UIProvider";

const parseNdjson = (text: string): Card[] =>
  text
    .split("\n")
    .map((line) => line.trim())
    .filter(Boolean)
    .map((line) => JSON.parse(line) as Card);

export default function ExportSection() {
  const { showToastMessage, setErrorMessage } = useUI();
  const [busy, setBusy] = useState(false);

  const handleExport = async () => {
    setBusy(true);
    try {
      const response = await fetch("/api/carddata");
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const cards = parseNdjson(await response.text());
      if (cards.length === 0) {
        showToastMessage("No card data to export.");
        return;
      }
      downloadCsv(cards);
      showToastMessage(`Exported ${cards.length} card${cards.length === 1 ? "" : "s"}.`);
    } catch (err) {
      setErrorMessage("Failed to export card data.");
      console.error(err);
    } finally {
      setBusy(false);
    }
  };

  return (
    <div className="card card-border bg-base-200">
      <div className="card-body">
        <h3 className="text-sm font-semibold uppercase tracking-wide opacity-70">
          Export
        </h3>
        <p className="text-sm opacity-80">
          Download all captured cards as a CSV file.
        </p>
        <div className="card-actions">
          <button
            type="button"
            className="btn btn-primary w-full gap-2"
            onClick={handleExport}
            disabled={busy}
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
                d="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5M16.5 12L12 16.5m0 0L7.5 12m4.5 4.5V3"
              />
            </svg>
            {busy ? "Preparing CSV…" : "Export Card Data (CSV)"}
          </button>
        </div>
      </div>
    </div>
  );
}
