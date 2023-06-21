import formatBytes from "./FormatBytes";

export default function LittleFSInfo({ tab, data }) {
  const little_fs_info_tab = 3;
  const opentab = tab;
  const { totalBytes: littlefsTotalBytes, usedBytes: littlefsUsedBytes } = data;
  const prettylittlefsinfototal = formatBytes(littlefsTotalBytes);
  const prettylittlefsinfoused = formatBytes(littlefsUsedBytes);

  return (
    <div
      id="tab_littlefsinfo"
      className={opentab === little_fs_info_tab ? "block" : "hidden"}
    >
      <div className="container">
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
    </div>
  );
}
