import React, { useState } from "react";
import DataTable from "../components/DataTable";

export default function Home() {
  const [filter, setFilter] = useState("");

  return (
    <div className="flex justify-center items-center pt-2">
      <div className="text-center w-full sm:w-auto">
        <div className="flex flex-col sm:flex-row justify-between items-center">
          <div className="prose sm:prose-xl px-2">
            <h4>Captured access card credentials are listed below 👇</h4>
          </div>
          <div className="divider divider-horizontal"></div>
          <div className="form-control pt-4 sm:pt-0">
            <input
              type="text"
              className="input input-bordered input-primary w-full sm:w-auto"
              placeholder="Search Card Number"
              value={filter}
              onChange={(event) => setFilter(event.target.value)}
            />
          </div>
        </div>
        <DataTable filter={filter} />
      </div>
    </div>
  );
}