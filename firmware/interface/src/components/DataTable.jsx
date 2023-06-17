import React, { useState, useEffect, useMemo } from "react";
import ndjsonStream from "can-ndjson-stream";
import Spinner from "./Spinner";
import ErrorAlert from "./ErrorAlert";

export default function DataTable({ filter }) {
  const [cardData, setCardCata] = useState([]);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState("");

  const streamerr = (error) => {
    setError("Failed to fetch card data - check console log 👀");
    console.error(error.message);
  };

  const getCardData = async () => {
    fetch("/api/carddata")
      .then((response) => {
        return ndjsonStream(response.body);
      })
      .then((cardDataStream) => {
        let read;
        let cardEntry = [];
        const reader = cardDataStream.getReader();
        reader
          .read()
          .then(function processValue({ done, value }) {
            if (done) {
              return cardEntry;
            }
            cardEntry.push(value);
            return reader.read().then(processValue, streamerr);
          }, streamerr)
          .then((data) => {
            setCardCata([...cardData, ...cardEntry]);
          });
      })
      .catch((error) => {
        setError("Failed to fetch card data - check console log 👀");
        console.error(error.message);
      });
    setIsLoading(false);
  };

  useEffect(() => {
    const intervalCall = setInterval(() => {
      getCardData();
    }, 5000);
    return () => {
      clearInterval(intervalCall);
    };
  }, []);

  const filteredCardData = useMemo(() =>
    cardData.filter((card) => {
      return card.card_number.toString().includes(Object.values({ filter }));
    })
  );

  const renderCardData = (
    <div className="overflow-x-auto">
      <table className="table w-full">
        <thead>
          <tr>
            <th>Bit Length</th>
            <th>Region Code</th>
            <th>Facility Code</th>
            <th>Card Number</th>
            <th>Issue Level</th>
            <th>Hex</th>
            <th>Raw</th>
          </tr>
        </thead>
        <tbody>
          {filteredCardData.map((item, index) => (
            <tr key={index}>
              <td>{item.bit_length}</td>
              <td>{item.region_code}</td>
              <td>{item.facility_code}</td>
              <td>{item.card_number}</td>
              <td>{item.issue_level}</td>
              <td className="font-mono">{item.hex}</td>
              <td>
              <label htmlFor={("modal_" + index)} className="btn btn-sm btn-outline btn-info">Show</label>
              <input type="checkbox" id={("modal_" + index)} className="modal-toggle" />
              <div className="modal">
                <div className="modal-box">
                  <h3 className="text-lg font-bold">Raw Data</h3>
                  <p className="py-4 font-mono">{item.raw}</p>
                </div>
                <label className="modal-backdrop" htmlFor={("modal_" + index)}>Close</label>
              </div>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );

  const renderNoCardData = (
    <div className="flex justify-center items-center pt-6">
      <div className="text-center">
        <div className="alert alert-info">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            fill="none"
            viewBox="0 0 24 24"
            className="stroke-current shrink-0 w-6 h-6"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth="2"
              d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"
            ></path>
          </svg>
          <span>No card data found.</span>
        </div>
      </div>
    </div>
  );

  return (
    <div className="container mx-auto pt-4">
      {isLoading && <Spinner />}
      {cardData.length > 0 ? renderCardData : renderNoCardData}
      {error && <ErrorAlert message={error} />}
    </div>
  );
}