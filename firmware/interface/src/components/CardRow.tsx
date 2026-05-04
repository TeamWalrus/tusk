import { useState } from "react";
import type { CardWithMeta } from "../helpers/useCards";
import { relativeTime } from "../helpers/relativeTime";
import { proxmarkCloneCommand } from "../helpers/proxmark";
import CopyButton from "./CopyButton";

interface Props {
  card: CardWithMeta;
  now: number;
  tag: string;
  onTagChange: (raw: string, tag: string) => void;
}

const FORMAT_BADGE: Record<string, string> = {
  hid: "badge-info",
  gallagher: "badge-warning",
  unknown: "badge-ghost",
};

const FRESH_WINDOW_MS = 30_000;

export default function CardRow({ card, now, tag, onTagChange }: Props) {
  const [expanded, setExpanded] = useState(false);
  const [tagDraft, setTagDraft] = useState(tag);

  const isFresh =
    typeof card.capturedAt === "number" &&
    now - card.capturedAt < FRESH_WINDOW_MS;
  const ageLabel =
    typeof card.capturedAt === "number"
      ? relativeTime(card.capturedAt, now)
      : null;

  const proxmark = proxmarkCloneCommand(card);

  const commitTag = () => {
    if (tagDraft !== tag) onTagChange(card.raw, tagDraft);
  };

  return (
    <div
      className={
        "card card-compact bg-base-200 transition-colors " +
        (isFresh ? "ring-2 ring-success ring-offset-2 ring-offset-base-100" : "")
      }
    >
      <button
        type="button"
        onClick={() => setExpanded((v) => !v)}
        className="card-body gap-2 text-left"
        aria-expanded={expanded}
      >
        <div className="flex items-center justify-between gap-2">
          <div className="text-xs uppercase opacity-60">
            {ageLabel ?? "captured earlier"}
            {tag ? <span className="ml-2 normal-case">· {tag}</span> : null}
          </div>
          <span
            className={
              "badge badge-sm font-semibold uppercase " +
              (FORMAT_BADGE[card.card_type] ?? "badge-ghost")
            }
          >
            {card.card_type}
          </span>
        </div>

        <div className="font-mono text-base font-semibold uppercase break-all">
          <span className="opacity-60 text-sm not-italic mr-1">Hex</span>
          {card.hex}
        </div>

        <div className="flex flex-wrap gap-x-4 gap-y-1 text-sm">
          <span>
            <span className="opacity-60">FC</span>{" "}
            <span className="font-mono">{card.facility_code}</span>
          </span>
          <span>
            <span className="opacity-60">CN</span>{" "}
            <span className="font-mono">{card.card_number}</span>
          </span>
          {card.region_code !== undefined && card.region_code !== 0 && (
            <span>
              <span className="opacity-60">RC</span>{" "}
              <span className="font-mono">{card.region_code}</span>
            </span>
          )}
          {card.issue_level !== undefined && card.issue_level !== 0 && (
            <span>
              <span className="opacity-60">IL</span>{" "}
              <span className="font-mono">{card.issue_level}</span>
            </span>
          )}
          <span className="ml-auto opacity-60">
            <span className="opacity-80">bits</span>{" "}
            <span className="font-mono">{card.bit_length}</span>
          </span>
        </div>
      </button>

      {expanded && (
        <div
          className="border-t border-base-300 px-4 pb-4 pt-3"
          onClick={(e) => e.stopPropagation()}
        >
          <div className="flex flex-wrap gap-2">
            <CopyButton label="Hex" value={card.hex.toUpperCase()} />
            <CopyButton label="FC" value={String(card.facility_code)} />
            <CopyButton label="CN" value={String(card.card_number)} />
            <CopyButton label="Raw bits" value={card.raw} />
            {proxmark && <CopyButton label="Proxmark" value={proxmark} />}
          </div>

          <div className="mt-3">
            <div className="text-xs uppercase opacity-60 mb-1">Tag</div>
            <input
              type="text"
              className="input input-sm w-full"
              placeholder="e.g. main lobby, elevator B2"
              value={tagDraft}
              onChange={(e) => setTagDraft(e.target.value)}
              onBlur={commitTag}
              onKeyDown={(e) => {
                if (e.key === "Enter") (e.target as HTMLInputElement).blur();
              }}
              maxLength={64}
            />
          </div>

          <div className="mt-3">
            <div className="text-xs uppercase opacity-60 mb-1">Raw bits</div>
            <div className="font-mono text-xs break-all opacity-80">
              {card.raw}
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
