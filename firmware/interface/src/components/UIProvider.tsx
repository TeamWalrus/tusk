import {
  createContext,
  useCallback,
  useContext,
  useMemo,
  useRef,
  useState,
  type ReactNode,
} from "react";

interface UIContextValue {
  toastMessage: string;
  errorMessage: string;
  showToastMessage: (message: string, durationMs?: number) => void;
  setErrorMessage: (message: string) => void;
  clearError: () => void;
}

const UIContext = createContext<UIContextValue | null>(null);

export function UIProvider({ children }: { children: ReactNode }) {
  const [toastMessage, setToastMessage] = useState("");
  const [errorMessage, setErrorMessage] = useState("");
  const toastTimeout = useRef<ReturnType<typeof setTimeout> | null>(null);

  const showToastMessage = useCallback(
    (message: string, durationMs = 4000) => {
      setToastMessage(message);
      if (toastTimeout.current) clearTimeout(toastTimeout.current);
      toastTimeout.current = setTimeout(() => setToastMessage(""), durationMs);
    },
    []
  );

  const clearError = useCallback(() => setErrorMessage(""), []);

  const value = useMemo<UIContextValue>(
    () => ({
      toastMessage,
      errorMessage,
      showToastMessage,
      setErrorMessage,
      clearError,
    }),
    [toastMessage, errorMessage, showToastMessage, clearError]
  );

  return <UIContext.Provider value={value}>{children}</UIContext.Provider>;
}

export function useUI(): UIContextValue {
  const ctx = useContext(UIContext);
  if (!ctx) {
    throw new Error("useUI must be used inside <UIProvider>");
  }
  return ctx;
}
