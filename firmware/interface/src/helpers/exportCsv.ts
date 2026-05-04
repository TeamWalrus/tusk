import type { Card } from "../types/api";

const CSV_HEADER = [
  "card_type",
  "bit_length",
  "facility_code",
  "card_number",
  "region_code",
  "issue_level",
  "hex",
  "raw",
];

const escape = (value: unknown): string => {
  if (value === undefined || value === null) return "";
  const s = String(value);
  if (/[",\n]/.test(s)) return `"${s.replace(/"/g, '""')}"`;
  return s;
};

export function cardsToCsv(cards: Card[]): string {
  const lines = [CSV_HEADER.join(",")];
  for (const c of cards) {
    lines.push(
      [
        c.card_type,
        c.bit_length,
        c.facility_code,
        c.card_number,
        c.region_code ?? "",
        c.issue_level ?? "",
        c.hex,
        c.raw,
      ]
        .map(escape)
        .join(",")
    );
  }
  return lines.join("\n");
}

export function downloadCsv(cards: Card[]): void {
  const csv = cardsToCsv(cards);
  const blob = new Blob([csv], { type: "text/csv;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  const stamp = new Date().toISOString().replace(/[:.]/g, "-");
  a.href = url;
  a.download = `tusk-cards-${stamp}.csv`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}
