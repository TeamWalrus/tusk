export default function ErrorAlert({ message }) {
  return (
    <div className="flex justify-center items-center py-6 px-2 md:px-4 lg:px-6">
      <div className="text-center">
        <div className="alert alert-error max-w-xs md:max-w-sm lg:max-w-md">
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
              d="M10 14l2-2m0 0l2-2m-2 2l-2-2m2 2l2 2m7-2a9 9 0 11-18 0 9 9 0 0118 0z"
            />
          </svg>
          <span className="text-xs md:text-sm lg:text-base">{message}</span>
        </div>
      </div>
    </div>
  );
}