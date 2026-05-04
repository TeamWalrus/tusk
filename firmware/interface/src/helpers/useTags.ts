import { useCallback, useEffect, useState } from "react";

const STORAGE_KEY = "tusk.cardTags";

type TagMap = Record<string, string>;

const read = (): TagMap => {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) return {};
    const parsed = JSON.parse(raw) as unknown;
    if (parsed && typeof parsed === "object") return parsed as TagMap;
  } catch (err) {
    console.warn("Failed to read tags from localStorage", err);
  }
  return {};
};

const write = (map: TagMap) => {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(map));
  } catch (err) {
    console.warn("Failed to persist tags to localStorage", err);
  }
};

export function useTags() {
  const [tags, setTags] = useState<TagMap>(() => read());

  // Cross-tab sync: pick up changes made in other browser tabs.
  useEffect(() => {
    const onStorage = (event: StorageEvent) => {
      if (event.key === STORAGE_KEY) setTags(read());
    };
    window.addEventListener("storage", onStorage);
    return () => window.removeEventListener("storage", onStorage);
  }, []);

  const setTag = useCallback((key: string, value: string) => {
    setTags((prev) => {
      const next: TagMap = { ...prev };
      const trimmed = value.trim();
      if (trimmed) next[key] = trimmed;
      else delete next[key];
      write(next);
      return next;
    });
  }, []);

  const removeTag = useCallback((key: string) => {
    setTags((prev) => {
      if (!(key in prev)) return prev;
      const next = { ...prev };
      delete next[key];
      write(next);
      return next;
    });
  }, []);

  return { tags, setTag, removeTag };
}
