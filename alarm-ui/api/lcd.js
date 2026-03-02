let lcdData = {
  line1: "SMART ALARM",
  line2: "--:-- --",
  status: "Offline"
};

export default function handler(req, res) {

  // GET → return data
  if (req.method === "GET") {
    return res.status(200).json(lcdData);
  }

  // POST → update data
  if (req.method === "POST") {
    const { line1, line2, status } = req.body;

    if (line1 !== undefined) lcdData.line1 = line1;
    if (line2 !== undefined) lcdData.line2 = line2;
    if (status !== undefined) lcdData.status = status;

    return res.status(200).json({ message: "Updated", lcdData });
  }

  return res.status(405).json({ message: "Method Not Allowed" });
}