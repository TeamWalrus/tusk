export default function InfoAlert({ message }) {
  return (
    <div className="flex justify-center items-center py-6 px-2 md:px-4 lg:px-6">
      <div className="text-center">
        <div className="alert alert-info max-w-xs md:max-w-sm lg:max-w-md">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            className="stroke-current shrink-0 h-6 w-6 md:h-8 md:w-8 lg:h-10 lg:w-10"
            fill="none"
            viewBox="0 0 24 24"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth="2"
              d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"
            />
          </svg>
          <span className="text-xs md:text-sm lg:text-base">{message}</span>
        </div>
      </div>
    </div>
  );
}