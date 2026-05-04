const SIZES = [
  "Bytes",
  "KiB",
  "MiB",
  "GiB",
  "TiB",
  "PiB",
  "EiB",
  "ZiB",
  "YiB",
] as const;

export default function formatBytes(
  bytes: number | undefined | null,
  decimals = 2
): string {
  if (!bytes) return "0 Bytes";

  const k = 1024;
  const dm = Math.max(decimals, 0);
  const i = Math.floor(Math.log(bytes) / Math.log(k));

  return `${parseFloat((bytes / Math.pow(k, i)).toFixed(dm))} ${SIZES[i]}`;
}
