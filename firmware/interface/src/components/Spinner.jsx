export default function Spinner() {
  return (
    <div className="flex items-center justify-center pb-4">
      <span className="loading loading-infinity loading-lg text-info"></span>
      <div className="pl-2">Fetching card data</div>
    </div>
  );
}
