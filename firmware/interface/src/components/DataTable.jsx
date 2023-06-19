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
                  <RawDataModal index={index} raw={item.raw} />
                </td>
              </tr>
            ))}
          </tbody>
        </table>
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
