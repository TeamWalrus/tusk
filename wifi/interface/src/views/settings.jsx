import React, { useState, useEffect } from 'react';

export default function Settings() {

    const [littlefsinfo, setLittleFSinfo] = useState([]);

    useEffect(() => {
    fetch("/api/littlefsinfo")
        .then((response) => response.json())
        .then((littlefsinfo) => {
            setLittleFSinfo(littlefsinfo);
        })
        .catch((err) => {
            console.log(err.message);
        });
    }, [])

    return (
        <div className="flex justify-center items-center">
            <div className="text-center">
                <article className="prose lg:prose-xl">
                    <h4>ESP32 LittleFS Info:</h4>
                    <div className="stats shadow">
                        <div className="stat place-items-center ">
                            <div className="stat-title text-info font-semibold">Total</div>
                            <div className="stat-value">{littlefsinfo.totalBytes} bytes</div>
                        </div>
                        <div className="stat place-items-center">
                            <div className="stat-title text-warning font-semibold">Used</div>
                            <div className="stat-value">{littlefsinfo.usedBytes} bytes</div>
                        </div>
                    </div>
                </article>
            </div>
        </div>
        );
}