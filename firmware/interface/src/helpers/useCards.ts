import { useEffect, useRef, useState } from "react";
import type { Card } from "../types/api";
import { useDeviceSettings } from "../components/DeviceSettingsProvider";

export interface CardWithMeta extends Card {
  capturedAt?: number;
  // Stable per-entry id used as a React key. Lets us tolerate duplicates in
  // debug mode (where the same physical card is intentionally captured many
  // times) without React key collisions.
  _id: number;
}

let nextId = 1;
const newId = () => nextId++;

// Used as the React key on each row. Falls back to a derived value if a
// migration ever produces an entry without an _id.
export const cardEntryKey = (card: CardWithMeta): string =>
  card._id ? String(card._id) : `${card.hex}-${card.raw}`;

export const cardKey = (card: Card) => `${card.hex}-${card.raw}`;

const parseNdjson = (text: string): Card[] =>
  text
    .split("\n")
    .map((line) => line.trim())
    .filter(Boolean)
    .map((line) => JSON.parse(line) as Card);

export function useCards() {
  const { deviceSettings } = useDeviceSettings();
  const [cards, setCards] = useState<CardWithMeta[]>([]);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState("");

  // Keep the latest debug_mode reachable from the long-lived SSE handler
  // closure without re-subscribing on every state change.
  const debugRef = useRef(deviceSettings.debug_mode);
  useEffect(() => {
    debugRef.current = deviceSettings.debug_mode;
  }, [deviceSettings.debug_mode]);

  useEffect(() => {
    let cancelled = false;

    fetch("/api/carddata")
      .then((response) => {
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        return response.text();
      })
      .then((text) => {
        if (cancelled) return;
        setCards(parseNdjson(text).map((c) => ({ ...c, _id: newId() })));
        setError("");
      })
      .catch((err) => {
        if (cancelled) return;
        setError("Failed to fetch card data.");
        console.error(err);
      })
      .finally(() => {
        if (!cancelled) setIsLoading(false);
      });

    const events = new EventSource("/events");
    events.addEventListener("card", (event: MessageEvent<string>) => {
      try {
        const card = JSON.parse(event.data) as Card;
        const captured: CardWithMeta = {
          ...card,
          capturedAt: Date.now(),
          _id: newId(),
        };
        setCards((prev) => {
          if (debugRef.current) {
            // No dedup in debug mode — show every read.
            return [captured, ...prev];
          }
          const key = cardKey(card);
          if (prev.some((c) => cardKey(c) === key)) return prev;
          return [captured, ...prev];
        });
      } catch (err) {
        console.error("Failed to parse SSE card payload", err);
      }
    });
    events.addEventListener("clear", () => setCards([]));
    events.onerror = () => {
      console.warn("SSE connection lost, attempting to reconnect");
    };

    return () => {
      cancelled = true;
      events.close();
    };
  }, []);

  return { cards, isLoading, error };
}
