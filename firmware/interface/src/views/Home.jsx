import React, { useState } from "react";
import DataTable from "../components/DataTable";

export default function Home() {
  const [filter, setFilter] = useState("");

  return (
    <div className="flex justify-center items-center pt-6">
      <div className="text-center">
        <div className="flex w-full justify-around items-center pb-5">
          <article className="prose lg:prose-xl">
            <h4>Captured access card credentials are listed below 👇</h4>
          </article>
          <div className="divider divider-horizontal"></div>
          <div className="form-control">
            <input
              type="text"
              className="input input-bordered input-primary"
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
