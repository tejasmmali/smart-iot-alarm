let alarm = { time: "07:30" };

export default function handler(req, res) {

  // SET alarm
  if (req.method === "POST") {
    alarm = req.body;
    return res.status(200).json({ ok: true });
  }

  // GET alarm
  res.status(200).json(alarm);
}