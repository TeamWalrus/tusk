import React, { useState } from "react";
import DataTable from "../components/DataTable";

export default function Home() {
  const [filter, setFilter] = useState("");

  return (
    <div>
      <div className="flex flex-col sm:flex-row sm:w-auto justify-center items-center">
        <div className="prose sm:prose-xl px-2">
          <h4 className="px-2">
            Captured access card credentials are listed below 👇
          </h4>
        </div>
        <div className="divider divider-horizontal"></div>
        <div className="form-control pt-4 sm:pt-0">
          <input
            type="text"
            className="input input-bordered input-primary w-full sm:w-auto px-2 text-xs md:text-sm lg:text-base text-center"
            placeholder="Search Card Number"
            value={filter}
            onChange={(event) => setFilter(event.target.value)}
          />
        </div>
      </div>
      <div className="w-full justify-between items-center px-4 sm:px-0">
        <DataTable filter={filter} />
      </div>
    </div>
  );
}
