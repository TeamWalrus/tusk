import React, { useState, useEffect, useMemo } from "react";
import ndjsonStream from "can-ndjson-stream";
import Spinner from "./Spinner";
import ErrorAlert from "./ErrorAlert";
import InfoAlert from "./InfoAlert";
import CardLogo from "./CardLogo";
import RawDataModal from "./RawDataModal";

export default function DataTable({ filter }) {
  const [cardData, setCardData] = useState([]);
  const [isLoading, setIsLoading] = useState(true);
  const [isFetching, setIsFetching] = useState(false);
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
    setIsFetching(true);
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
    setIsFetching(false);
    setIsLoading(false);
  };

  useEffect(() => {
    const intervalCall = setInterval(() => {
      if (!isFetching) {
        getCardData();
      }
    }, 5000);
    return () => {
      clearInterval(intervalCall);
    };
  }, [isFetching]);

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
    return <CardLogo cardType={cardType} />;
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
    <div className="container">
      {sortedCardData.length > 0 ? (
        <div className="container">
          <div className="overflow-x-auto rounded-lg shadow hidden md:block">
            <table className="table w-full">
              <thead className="border-b-2">
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
              <tbody className="divide-y">
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
                      <RawDataModal index={index} raw={item.raw} />
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <div className="grid p-2 grid-cols-1 gap-4 md:hidden">
            {sortedCardData.map((item, index) => (
              <div className="space-y-3 p-4 rounded-lg shadow">
                <div className="flex items-center space-x-4 text-sm">
                  <div>Bit: {item.bit_length}</div>
                  <div>Hex: {item.hex}</div>
                  <div>
                    <span
                      className={
                        "p-1.5 text-xs font-bold uppercase tracking-wider rounded-lg " +
                        (item.card_type === "hid"
                          ? "bg-blue-700 text-white"
                          : "bg-yellow-400 text-black")
                      }
                    >
                      {item.card_type}
                    </span>
                  </div>
                </div>
                <div className="flex items-center space-x-4 text-sm">
                  {item.region_code && (
                    <div className="text-sm">RC: {item.region_code}</div>
                  )}
                  <div className="text-sm">FC: {item.facility_code}</div>
                  <div className="text-sm">CN: {item.card_number}</div>
                  {item.issue_level && (
                    <div className="text-sm">IL: {item.issue_level}</div>
                  )}
                </div>
              </div>
            ))}
          </div>
        </div>
      ) : (
        <InfoAlert message="No card data found." />
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
