import { useState } from "react";
import DataTable from "../components/DataTable";
import { useCards } from "../helpers/useCards";
import { useTags } from "../helpers/useTags";

export default function Home() {
  const [filter, setFilter] = useState("");
  const { cards, isLoading, error } = useCards();
  const { tags, setTag } = useTags();

  return (
    <>
      <div className="px-3 pt-3">
        <label className="input input-sm w-full">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            viewBox="0 0 24 24"
            fill="none"
            stroke="currentColor"
            strokeWidth={2}
            className="h-4 w-4 opacity-60"
          >
            <circle cx="11" cy="11" r="7" />
            <path strokeLinecap="round" d="M21 21l-4.3-4.3" />
          </svg>
          <input
            type="search"
            placeholder="Filter by hex, FC, CN, or tag"
            value={filter}
            onChange={(event) => setFilter(event.target.value)}
            className="grow"
          />
        </label>
      </div>

      <DataTable
        cards={cards}
        isLoading={isLoading}
        error={error}
        filter={filter}
        tags={tags}
        onTagChange={setTag}
      />
    </>
  );
}
