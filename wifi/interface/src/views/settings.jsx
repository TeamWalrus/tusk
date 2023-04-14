import React, { useState, useEffect } from 'react';
import formatBytes from "../components/FormatBytes";

export default function Settings() {

    const [littlefsinfo, setLittleFSinfo] = useState([]);
    const [sdcardinfo, setSDCardInfo] = useState([]);
    const [error, setError] = useState("");

    const getLittleFSInfo = async () => {
        fetch("/api/littlefsinfo")
            .then((response) => response.json())
            .then((littlefsinfo) => {
                setLittleFSinfo(littlefsinfo);
            })
            .catch((error) => {
                setError(error);
            });
    }

    const getSDCardInfo = async () => {
        fetch("/api/sdcardinfo")
            .then((response) => response.json())
            .then((sdcardinfo) => {
                setSDCardInfo(sdcardinfo);
            })
            .catch((error) => {
                setError(error);
            });
    }

    useEffect(() => {
        getLittleFSInfo();
        getSDCardInfo();
    }, [])

    const prettylittlefsinfototal = formatBytes(littlefsinfo.totalBytes);
    const prettylittlefsinfoused = formatBytes(littlefsinfo.usedBytes);
    const renderLittleFSInfo = (
        <div className="container">
            <h4>ESP32 LittleFS Info:</h4>
            <div className="stats shadow">
                <div className="stat place-items-center ">
                    <div className="stat-title text-info font-semibold">Total</div>
                    <div className="stat-value">{prettylittlefsinfototal}</div>
                </div>
                <div className="stat place-items-center">
                    <div className="stat-title text-warning font-semibold">Used</div>
                    <div className="stat-value">{prettylittlefsinfoused}</div>
                </div>
            </div>
        </div>
    );

    const prettysdcardinfototal = formatBytes(sdcardinfo.totalBytes);
    const prettysdcardinfoused = formatBytes(sdcardinfo.usedBytes);
    const renderSDCardInfo = (
        <div className="container">
            <h4>SD Card Info:</h4>
            <div className="stats shadow">
                <div className="stat place-items-center ">
                    <div className="stat-title text-info font-semibold">Total</div>
                    <div className="stat-value">{prettysdcardinfototal}</div>
                </div>
                <div className="stat place-items-center">
                    <div className="stat-title text-warning font-semibold">Used</div>
                    <div className="stat-value">{prettysdcardinfoused}</div>
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
                        <span>{error}</span>
                    </div>
                </div>
            </div>
        </div>
    );
    
    return (
        <div className="flex justify-center items-center">
            <div className="text-center">
                <article className="prose lg:prose-xl">
                    {littlefsinfo && renderLittleFSInfo}
                    {sdcardinfo && renderSDCardInfo}
                    {error && renderError}
                </article>
            </div>
        </div>
        );
}