import { useUI } from "./UIProvider";

export default function Toast() {
  const { toastMessage } = useUI();
  if (!toastMessage) return null;
  return (
    <div className="toast-center toast toast-top">
      <div className="alert alert-success">
        <span>{toastMessage}</span>
      </div>
    </div>
  );
}
