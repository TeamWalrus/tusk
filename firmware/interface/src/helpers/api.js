const postApiRequest = async (url, data = {}) => {
  try {
    const response = await fetch(url, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(data),
    });

    if (response.status !== 200) {
      throw new Error(
        `Network Error: ${response.status}, ${response.statusText}`
      );
    }

    return response.text();
  } catch (error) {
    console.error(
      "API Error - Check console logs for additional information.",
      error
    );
    throw error;
  }
};

const fetchApiRequest = async (url) => {
  try {
    const response = await fetch(url);
    if (response.status !== 200) {
      throw new Error(
        `Network Error: ${response.status}, ${response.statusText}`
      );
    }

    return await response.json();
  } catch (error) {
    console.error(
      "API Error - Check console logs for additional information.",
      error
    );
    throw error;
  }
};

export { postApiRequest, fetchApiRequest };
