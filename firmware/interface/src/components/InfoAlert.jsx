export default function InfoAlert({ message }) {
  return (
    <div className="flex items-center justify-center p-2 sm:p-4">
      <div className="text-center">
        <div className="alert alert-info max-w-sm lg:max-w-md">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            className="h-6 w-6 shrink-0 stroke-current md:h-8 md:w-8 lg:h-10 lg:w-10"
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
          <span className="text-sm lg:text-base">{message}</span>
        </div>
      </div>
    </div>
  );
}
