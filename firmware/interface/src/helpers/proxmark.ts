import type { Card } from "../types/api";

// Build a Proxmark3 LF clone command for HID-formatted cards.
// Reference: `lf hid clone -w <format> --fc <fc> --cn <cn> [--il <il>]`
export function proxmarkCloneCommand(card: Card): string | null {
  if (card.card_type !== "hid") return null;

  const formatByBits: Record<number, string> = {
    26: "H10301",
    35: "C1k35s",
    37: "H10302",
    48: "C1k48s",
  };
  const format = formatByBits[card.bit_length] ?? `bits${card.bit_length}`;

  const parts = [
    "lf hid clone",
    `-w ${format}`,
    `--fc ${card.facility_code}`,
    `--cn ${card.card_number}`,
  ];
  if (card.issue_level && card.issue_level !== 0) {
    parts.push(`--il ${card.issue_level}`);
  }
  return parts.join(" ");
}
