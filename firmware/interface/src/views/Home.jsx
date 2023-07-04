import React, { useState } from "react";
import DataTable from "../components/DataTable";

export default function Home() {
  const [filter, setFilter] = useState("");

  return (
    <div>
      <div className="flex flex-col items-center justify-center sm:w-auto sm:flex-row">
        <div className="prose px-2 sm:prose-xl">
          <h4>Captured access card credentials are listed below 👇</h4>
        </div>
        <div className="divider divider-horizontal"></div>
        <div className="form-control pt-4 sm:pt-0">
          <input
            type="text"
            className="input-bordered input-primary input w-full px-2 text-center text-sm sm:w-auto md:text-base"
            placeholder="Search Card Number"
            value={filter}
            onChange={(event) => setFilter(event.target.value)}
          />
        </div>
      </div>
      <div className="w-full items-center justify-between px-4 sm:px-0">
        <DataTable filter={filter} />
      </div>
    </div>
  );
}
