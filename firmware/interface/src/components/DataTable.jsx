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

  const SortableIndicator = (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      class="ml-1 h-3 w-3"
      aria-hidden="true"
      fill="currentColor"
      viewBox="0 0 320 512"
    >
      <path d="M27.66 224h264.7c24.6 0 36.89-29.78 19.54-47.12l-132.3-136.8c-5.406-5.406-12.47-8.107-19.53-8.107c-7.055 0-14.09 2.701-19.45 8.107L8.119 176.9C-9.229 194.2 3.055 224 27.66 224zM292.3 288H27.66c-24.6 0-36.89 29.77-19.54 47.12l132.5 136.8C145.9 477.3 152.1 480 160 480c7.053 0 14.12-2.703 19.53-8.109l132.3-136.8C329.2 317.8 316.9 288 292.3 288z" />
    </svg>
  );

  const renderSortIndicator = (column) => {
    if (column === sortColumn) {
      return (
        <span className="ps-[3px]">{sortDirection === "asc" ? "▲" : "▼"}</span>
      );
    }
    return SortableIndicator;
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
    <div>
      {sortedCardData.length > 0 ? (
        <div>
          <div className="hidden overflow-x-auto md:block">
            <table className="table w-full">
              <thead className="border-b-2 uppercase">
                <tr>
                  <th>Type</th>
                  <th
                    className="cursor-pointer"
                    onClick={() => handleSort("bit_length")}
                  >
                    <div class="flex items-center">
                      Bit Length {renderSortIndicator("bit_length")}
                    </div>
                  </th>
                  <th
                    className="cursor-pointer"
                    onClick={() => handleSort("region_code")}
                  >
                    <div class="flex items-center">
                      Region Code {renderSortIndicator("region_code")}
                    </div>
                  </th>
                  <th
                    className="cursor-pointer"
                    onClick={() => handleSort("facility_code")}
                  >
                    <div class="flex items-center">
                      Facility Code {renderSortIndicator("facility_code")}
                    </div>
                  </th>
                  <th
                    className="cursor-pointer"
                    onClick={() => handleSort("card_number")}
                  >
                    <div class="flex items-center">
                      Card Number {renderSortIndicator("card_number")}
                    </div>
                  </th>
                  <th
                    className="cursor-pointer"
                    onClick={() => handleSort("issue_level")}
                  >
                    <div class="flex items-center">
                      Issue Level {renderSortIndicator("issue_level")}
                    </div>
                  </th>
                  <th
                    className="cursor-pointer"
                    onClick={() => handleSort("hex")}
                  >
                    <div class="flex items-center">
                      Hex {renderSortIndicator("hex")}
                    </div>
                  </th>
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
                    <td className="font-mono uppercase">{item.hex}</td>
                    <td>
                      <RawDataModal index={index} raw={item.raw} />
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <div className="grid grid-cols-1 gap-4 p-2 md:hidden">
            {sortedCardData.map((item) => (
              <div className="space-y-3 rounded-lg border p-4">
                <div className="flex w-full items-center text-sm">
                  <div className="flex w-full items-center">
                    <div className="text-sm font-semibold">Hex:</div>
                    <div className="text-sm uppercase pl-0.5">{item.hex}</div>
                  </div>
                  <div
                    className={
                      "badge font-semibold uppercase " +
                      (item.card_type === "hid"
                        ? "bg-blue-700 text-white"
                        : "bg-amber-500 text-black")
                    }
                  >
                    {item.card_type}
                  </div>
                </div>
                <div className="flex items-center space-x-4 text-sm">
                  <div className="flex">
                    <div className="text-sm font-semibold">FC:</div>
                    <div className="text-sm pl-0.5">{item.facility_code}</div>
                  </div>
                  <div className="flex">
                    <div className="text-sm font-semibold">CN:</div>
                    <div className="text-sm pl-0.5">{item.card_number}</div>
                  </div>
                  {item.region_code && (
                    <div className="flex">
                      <div className="text-sm font-semibold">RC:</div>
                      <div className="text-sm pl-0.5">{item.region_code}</div>
                    </div>
                  )}
                  {item.issue_level && (
                    <div className="flex">
                      <div className="text-sm font-semibold">IL:</div>
                      <div className="text-sm pl-0.5">{item.issue_level}</div>
                    </div>
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
