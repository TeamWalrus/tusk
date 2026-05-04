import { useMemo } from "react";
import Spinner from "./Spinner";
import ErrorAlert from "./ErrorAlert";
import InfoAlert from "./InfoAlert";
import CardRow from "./CardRow";
import type { CardWithMeta } from "../helpers/useCards";
import { cardEntryKey } from "../helpers/useCards";
import { useNow } from "../helpers/useNow";

interface Props {
  cards: CardWithMeta[];
  isLoading: boolean;
  error: string;
  filter: string;
  tags: Record<string, string>;
  onTagChange: (raw: string, tag: string) => void;
}

export default function DataTable({
  cards,
  isLoading,
  error,
  filter,
  tags,
  onTagChange,
}: Props) {
  const now = useNow(5000);

  const filtered = useMemo(() => {
    if (!filter) return cards;
    const q = filter.toLowerCase();
    return cards.filter((c) => {
      const tag = tags[c.raw]?.toLowerCase() ?? "";
      return (
        c.card_number.toString().includes(q) ||
        c.hex.toLowerCase().includes(q) ||
        c.facility_code.toString().includes(q) ||
        tag.includes(q)
      );
    });
  }, [cards, filter, tags]);

  if (isLoading) {
    return (
      <div className="container mx-auto pt-4">
        <Spinner />
        {error && <ErrorAlert message={error} />}
      </div>
    );
  }

  return (
    <div className="container mx-auto px-2 pt-3 pb-8 sm:max-w-2xl">
      {filtered.length === 0 ? (
        <InfoAlert
          message={
            cards.length === 0
              ? "No captures yet. Present a card to the reader."
              : "No matches for that filter."
          }
        />
      ) : (
        <div className="flex flex-col gap-3">
          {filtered.map((card) => (
            <CardRow
              key={cardEntryKey(card)}
              card={card}
              now={now}
              tag={tags[card.raw] ?? ""}
              onTagChange={onTagChange}
            />
          ))}
        </div>
      )}
      {error && <ErrorAlert message={error} />}
    </div>
  );
}
