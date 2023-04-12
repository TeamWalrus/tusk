import React, { useState, useEffect } from 'react';
import ndjsonStream from 'can-ndjson-stream';
import Spinner from './Spinner';

export default function DataTable() {

    const [cardData, setCardCata] = useState([]);
    const [isLoading, setIsLoading] = useState(true);
    const [error, setError] = useState("");


    const getCardData = async () => {
        try {
            const response = await fetch("/api/carddata");
            if (!response.ok) {
                const message = `An error has occured: ${response.status}`;
                throw new Error(message);
            }
            const dataReader = ndjsonStream(response.body).getReader();
            // let cardData = await response.json();
            // setCardCata(cardData.entries);
            let result;
            while (!result || !result.done) {
                result = await dataReader.read();
                console.log(result.done, result.value);
                //setCardCata(cardData => cardData.concat(result.value));
                //console.log(cardData);
            }
            setError("");
        } catch(error) {
            setError(error.message);
            console.log(error.message);
        } finally {
            setIsLoading(false);
        }  
    }

    useEffect(() => {
        const intervalCall = setInterval(() => {
            getCardData();
        }, 5000);
        return () => {
            clearInterval(intervalCall);
        };
    }, []);


    const renderCardData = (
    <div className="overflow-x-auto">
        <table className="table w-full">
            <thead>
                <tr>
                <th>Bit Length</th>
                <th>Facility Code</th>
                <th>Card Number</th>
                <th>Hex</th>
                <th>Raw</th>
                </tr>
            </thead>
            <tbody>
                {cardData.map((item, index) => (
                <tr key={index}>
                    <td>{item.bit_length}</td>
                    <td>{item.facility_code}</td>
                    <td>{item.card_number}</td>
                    <td>{item.hex}</td>
                    <td>{item.raw}</td>
                </tr>
                ))}
            </tbody>
        </table>
    </div>
    );

    const renderNoCardData = (
        <div className="flex justify-center items-center pt-6">
            <div className="text-center">
                <div className="alert alert-info shadow-lg">
                    <div>
                        <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" className="stroke-current flex-shrink-0 w-6 h-6"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path></svg>
                        <span>No card data found.</span>
                    </div>
                </div>
            </div>
        </div>
    );

    const renderError = (
        <div className="flex justify-center items-center pt-6">
            <div className="text-center">
                <div className="alert alert-error shadow-lg">
                    <div>
                        <svg xmlns="http://www.w3.org/2000/svg" className="stroke-current flex-shrink-0 h-6 w-6" fill="none" viewBox="0 0 24 24"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth="2" d="M10 14l2-2m0 0l2-2m-2 2l-2-2m2 2l2 2m7-2a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>
                        <span>Error: {error}</span>
                    </div>
                </div>
            </div>
        </div>
    );

    
    return (
        <div className="container mx-auto pt-4">
            {isLoading && <Spinner />}
            {cardData ? renderCardData : renderNoCardData}
            {error && renderError}
        </div>
  )
}