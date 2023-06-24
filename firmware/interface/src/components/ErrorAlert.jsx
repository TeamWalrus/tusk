export default function ErrorAlert({ message }) {
  return (
    <div className="flex items-center justify-center px-2 py-6 md:px-4 lg:px-6">
      <div className="text-center">
        <div className="alert alert-error max-w-xs md:max-w-sm lg:max-w-md">
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
              d="M10 14l2-2m0 0l2-2m-2 2l-2-2m2 2l2 2m7-2a9 9 0 11-18 0 9 9 0 0118 0z"
            />
          </svg>
          <span className="text-xs md:text-sm lg:text-base">{message}</span>
        </div>
      </div>
    </div>
  );
}
