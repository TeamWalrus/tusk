import React, { useState, useEffect, useMemo } from "react";
import ndjsonStream from "can-ndjson-stream";
import Spinner from "./Spinner";
import ErrorAlert from "./ErrorAlert";
import hidLogo from "../assets/hid_logo.png";
import gallagherLogo from "../assets/gallagher_logo.png";

export default function DataTable({ filter }) {
  const [cardData, setCardData] = useState([]);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState("");
  const [sortColumn, setSortColumn] = useState("");
  const [sortDirection, setSortDirection] = useState("");

  const streamerr = (error) => {
    setError(
      "Failed to fetch card data. Check console logs for additional information."
    );
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
            setCardData([...cardData, ...cardEntry]);
          });
      })
      .catch((error) => {
        setError(
          "Failed to fetch card data. Check console logs for additional information."
        );
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

  const handleSort = (column) => {
    if (column === sortColumn) {
      setSortDirection(sortDirection === "asc" ? "desc" : "asc");
    } else {
      setSortColumn(column);
      setSortDirection("asc");
    }
  };

  const renderSortIndicator = (column) => {
    if (column === sortColumn) {
      return <span>{sortDirection === "asc" ? "▲" : "▼"}</span>;
    }
    return null;
  };

  const renderCardTypeImage = (cardType) => {
    switch (cardType) {
      case "hid":
        return (
          <img
            className="h-auto max-w-xs mx-auto"
            width="80px"
            src={hidLogo}
            alt="hid-logo"
          />
        );
      case "gallagher":
        return (
          <img
            className="h-auto max-w-xs mx-auto"
            width="120px"
            src={gallagherLogo}
            alt="gallagher-logo"
          />
        );
    }
  };

  const filteredCardData = useMemo(
    () =>
      cardData.filter((card) => card.card_number.toString().includes(filter)),
    [cardData, filter]
  );

  const sortedCardData = useMemo(() => {
    if (sortColumn && sortDirection) {
      const sortedData = [...filteredCardData].sort((a, b) => {
        if (sortDirection === "asc") {
          return a[sortColumn] - b[sortColumn];
        } else {
          return b[sortColumn] - a[sortColumn];
        }
      });
      return sortedData;
    }
    return filteredCardData;
  }, [filteredCardData, sortColumn, sortDirection]);

  const renderCardData = (
    <div className="overflow-x-auto">
      {sortedCardData.length > 0 ? (
        <table className="table w-full">
          <thead>
            <tr>
              <th>Type</th>
              <th onClick={() => handleSort("bit_length")}>
                Bit Length {renderSortIndicator("bit_length")}
              </th>
              <th onClick={() => handleSort("region_code")}>
                Region Code {renderSortIndicator("region_code")}
              </th>
              <th onClick={() => handleSort("facility_code")}>
                Facility Code {renderSortIndicator("facility_code")}
              </th>
              <th onClick={() => handleSort("card_number")}>
                Card Number {renderSortIndicator("card_number")}
              </th>
              <th onClick={() => handleSort("issue_level")}>
                Issue Level {renderSortIndicator("issue_level")}
              </th>
              <th>Hex</th>
              <th>Raw</th>
            </tr>
          </thead>
          <tbody>
            {sortedCardData.map((item, index) => (
              <tr key={index}>
                <td>{renderCardTypeImage(item.card_type)}</td>
                <td>{item.bit_length}</td>
                <td>{item.region_code}</td>
                <td>{item.facility_code}</td>
                <td>{item.card_number}</td>
                <td>{item.issue_level}</td>
                <td className="font-mono">{item.hex}</td>
                <td>
                  <label
                    htmlFor={"rawdata_modal_" + index}
                    className="btn btn-sm btn-outline btn-info"
                  >
                    Show
                  </label>
                  <input
                    type="checkbox"
                    id={"rawdata_modal_" + index}
                    className="modal-toggle"
                  />
                  <div className="modal">
                    <div className="modal-box">
                      <h3 className="text-lg font-bold">Raw Data</h3>
                      <p className="py-4 font-mono">{item.raw}</p>
                    </div>
                    <label
                      className="modal-backdrop"
                      htmlFor={"rawdata_modal_" + index}
                    >
                      Close
                    </label>
                  </div>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      ) : (
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
      )}
    </div>
  );

  return (
    <div className="container mx-auto pt-4">
      {isLoading ? <Spinner /> : renderCardData}
      {error && <ErrorAlert message={error} />}
    </div>
  );
}
