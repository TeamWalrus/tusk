const ensureOk = (response: Response): Response => {
  if (!response.ok) {
    throw new Error(`Network Error: ${response.status}, ${response.statusText}`);
  }
  return response;
};

export async function fetchApiRequest<T>(url: string): Promise<T> {
  try {
    const response = await fetch(url);
    return (await ensureOk(response).json()) as T;
  } catch (error) {
    console.error(
      "API Error - Check console logs for additional information.",
      error
    );
    throw error;
  }
}

export async function postApiRequest(
  url: string,
  data: Record<string, unknown> = {}
): Promise<string> {
  try {
    const response = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(data),
    });
    return await ensureOk(response).text();
  } catch (error) {
    console.error(
      "API Error - Check console logs for additional information.",
      error
    );
    throw error;
  }
}

type FormFields = Record<string, string | number | boolean>;

const toSearchParams = (data: FormData | FormFields): URLSearchParams => {
  if (data instanceof FormData) {
    const params = new URLSearchParams();
    data.forEach((value, key) => params.append(key, String(value)));
    return params;
  }
  return new URLSearchParams(
    Object.fromEntries(
      Object.entries(data).map(([k, v]) => [k, String(v)])
    )
  );
};

export async function postFormApiRequest(
  url: string,
  data: FormData | FormFields
): Promise<string> {
  try {
    const response = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/x-www-form-urlencoded" },
      body: toSearchParams(data),
    });
    return await ensureOk(response).text();
  } catch (error) {
    console.error(
      "API Error - Check console logs for additional information.",
      error
    );
    throw error;
  }
}
