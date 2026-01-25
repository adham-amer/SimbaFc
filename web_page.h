#pragma once

const char kIndexHtml[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SimbaFc Config</title>
  <style>
    :root {
      --bg: #f3efe5;
      --bg-accent: #f7d8b5;
      --ink: #2c2620;
      --muted: #6b5b4b;
      --card: #fffaf2;
      --stroke: #e5d3bb;
      --accent: #b04528;
      --accent-2: #2a6c6c;
      --shadow: rgba(87, 58, 28, 0.15);
      --bar: #cc6a3b;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: "Trebuchet MS", "Lucida Sans Unicode", "Lucida Grande", sans-serif;
      color: var(--ink);
      background:
        radial-gradient(circle at 20% 10%, rgba(255, 255, 255, 0.7), transparent 55%),
        radial-gradient(circle at 80% 0%, rgba(255, 214, 170, 0.7), transparent 45%),
        linear-gradient(135deg, var(--bg), var(--bg-accent));
      min-height: 100vh;
      padding: 24px;
    }
    header {
      display: flex;
      align-items: baseline;
      justify-content: space-between;
      margin-bottom: 24px;
    }
    h1 {
      margin: 0;
      font-size: 28px;
      letter-spacing: 0.04em;
    }
    .sub {
      color: var(--muted);
      font-size: 14px;
    }
    .grid {
      display: grid;
      gap: 16px;
      grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
    }
    .card {
      background: var(--card);
      border: 1px solid var(--stroke);
      border-radius: 16px;
      padding: 16px;
      box-shadow: 0 10px 24px var(--shadow);
      animation: rise 0.5s ease-out;
    }
    @keyframes rise {
      from { opacity: 0; transform: translateY(8px); }
      to { opacity: 1; transform: translateY(0); }
    }
    .card h2 {
      margin: 0 0 12px 0;
      font-size: 16px;
      text-transform: uppercase;
      letter-spacing: 0.14em;
      color: var(--accent);
    }
    .metric {
      display: flex;
      justify-content: space-between;
      padding: 6px 0;
      border-bottom: 1px dashed var(--stroke);
      font-size: 15px;
    }
    .metric:last-child { border-bottom: none; }
    .value {
      font-weight: 700;
      color: var(--accent-2);
    }
    .channels {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 8px;
      font-size: 12px;
    }
    .ch {
      display: grid;
      gap: 6px;
      background: rgba(255, 255, 255, 0.6);
      border: 1px solid var(--stroke);
      border-radius: 10px;
      padding: 8px;
    }
    .bar {
      position: relative;
      height: 10px;
      border-radius: 999px;
      background: #efe2d2;
      overflow: hidden;
    }
    .bar span {
      position: absolute;
      inset: 0;
      width: 50%;
      background: linear-gradient(90deg, var(--bar), #e29a62);
      transition: width 0.2s ease;
    }
    .gauge {
      width: 100%;
      max-width: 320px;
      margin: 0 auto 8px auto;
      display: block;
      border-radius: 12px;
      background: #f8efe3;
      border: 1px solid var(--stroke);
    }
    form {
      display: grid;
      gap: 10px;
    }
    .row {
      display: grid;
      gap: 10px;
      grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
    }
    label {
      display: grid;
      gap: 4px;
      font-size: 12px;
      color: var(--muted);
    }
    input {
      padding: 8px;
      border-radius: 8px;
      border: 1px solid var(--stroke);
      background: #ffffff;
      font-size: 14px;
    }
    button {
      padding: 10px 14px;
      border: none;
      border-radius: 10px;
      background: var(--accent);
      color: #ffffff;
      font-weight: 700;
      letter-spacing: 0.04em;
      cursor: pointer;
    }
    .status {
      font-size: 12px;
      color: var(--muted);
    }
  </style>
</head>
<body>
  <header>
    <h1>SimbaFc</h1>
    <div class="sub">Live attitude, channels, and tuning</div>
  </header>

  <div class="grid">
    <section class="card">
      <h2>Attitude</h2>
      <canvas id="attGauge" class="gauge" width="320" height="200"></canvas>
      <div class="metric"><span>Roll</span><span class="value" id="roll">0.00</span></div>
      <div class="metric"><span>Pitch</span><span class="value" id="pitch">0.00</span></div>
      <div class="metric"><span>Yaw</span><span class="value" id="yaw">0.00</span></div>
      <div class="metric"><span>Desired Roll</span><span class="value" id="droll">0.00</span></div>
      <div class="metric"><span>Desired Pitch</span><span class="value" id="dpitch">0.00</span></div>
      <div class="metric"><span>Roll Cmd</span><span class="value" id="rcmd">0.00</span></div>
      <div class="metric"><span>Pitch Cmd</span><span class="value" id="pcmd">0.00</span></div>
    </section>

    <section class="card">
      <h2>Channels</h2>
      <div class="channels" id="channels"></div>
    </section>

    <section class="card">
      <h2>Config</h2>
      <form id="configForm">
        <div class="row">
          <label>Roll Kp <input name="rollKp" step="0.01" type="number"></label>
          <label>Roll Ki <input name="rollKi" step="0.01" type="number"></label>
          <label>Roll Kd <input name="rollKd" step="0.01" type="number"></label>
        </div>
        <div class="row">
          <label>Pitch Kp <input name="pitchKp" step="0.01" type="number"></label>
          <label>Pitch Ki <input name="pitchKi" step="0.01" type="number"></label>
          <label>Pitch Kd <input name="pitchKd" step="0.01" type="number"></label>
        </div>
        <div class="row">
          <label>Max Attitude <input name="maxAttitudeDeg" step="0.1" type="number"></label>
          <label>Rate Deg/S <input name="mode1RateDegPerSec" step="0.1" type="number"></label>
          <label>Switch Thresh <input name="mode3StickThreshold" step="0.01" type="number"></label>
        </div>
        <div class="row">
          <label>PID Min <input name="pidOutputMin" step="0.1" type="number"></label>
          <label>PID Max <input name="pidOutputMax" step="0.1" type="number"></label>
        </div>
        <button type="submit">Apply Config</button>
        <div class="status" id="status">Waiting for data...</div>
      </form>
    </section>
  </div>

  <script>
    const channelWrap = document.getElementById("channels");
    for (let i = 0; i < 16; i++) {
      const div = document.createElement("div");
      div.className = "ch";
      div.innerHTML = "<div>CH" + (i + 1) + ": <span id='chv" + i + "'>0</span></div>" +
        "<div class='bar'><span id='chb" + i + "'></span></div>";
      channelWrap.appendChild(div);
    }

    const canvas = document.getElementById("attGauge");
    const ctx = canvas.getContext("2d");
    let lastRoll = 0;
    let lastPitch = 0;

    function setValue(id, value) {
      document.getElementById(id).textContent = value;
    }

    function lerp(a, b, t) {
      return a + (b - a) * t;
    }

    function drawGauge(rollDeg, pitchDeg) {
      const w = canvas.width;
      const h = canvas.height;
      const cx = w / 2;
      const cy = h / 2;
      ctx.clearRect(0, 0, w, h);

      ctx.fillStyle = "#fdf7ee";
      ctx.fillRect(0, 0, w, h);

      ctx.save();
      ctx.translate(cx, cy);
      ctx.rotate((-rollDeg * Math.PI) / 180);

      const pitchPx = (pitchDeg / 45) * (h * 0.3);
      ctx.translate(0, pitchPx);

      const grd = ctx.createLinearGradient(0, -h, 0, h);
      grd.addColorStop(0, "#9cc7d7");
      grd.addColorStop(0.5, "#fdf7ee");
      grd.addColorStop(1, "#d8a57a");
      ctx.fillStyle = grd;
      ctx.fillRect(-w, -h, w * 2, h * 2);

      ctx.strokeStyle = "#8c6a4d";
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.moveTo(-w, 0);
      ctx.lineTo(w, 0);
      ctx.stroke();

      for (let i = -2; i <= 2; i++) {
        const y = i * 20;
        ctx.beginPath();
        ctx.moveTo(-40, y);
        ctx.lineTo(40, y);
        ctx.stroke();
      }

      ctx.restore();

      ctx.strokeStyle = "#2c2620";
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.moveTo(cx - 40, cy);
      ctx.lineTo(cx + 40, cy);
      ctx.stroke();
      ctx.beginPath();
      ctx.moveTo(cx, cy - 10);
      ctx.lineTo(cx, cy + 10);
      ctx.stroke();
    }

    async function fetchData() {
      try {
        const response = await fetch("/data");
        const data = await response.json();
        setValue("roll", data.roll.toFixed(2));
        setValue("pitch", data.pitch.toFixed(2));
        setValue("yaw", data.yaw.toFixed(2));
        setValue("droll", data.desiredRoll.toFixed(2));
        setValue("dpitch", data.desiredPitch.toFixed(2));
        setValue("rcmd", data.rollCmd.toFixed(2));
        setValue("pcmd", data.pitchCmd.toFixed(2));
        if (Array.isArray(data.ch)) {
          data.ch.forEach((val, idx) => {
            const valEl = document.getElementById("chv" + idx);
            const barEl = document.getElementById("chb" + idx);
            if (valEl) valEl.textContent = val;
            if (barEl) {
              const pct = Math.max(0, Math.min(1, (val - 172) / (1811 - 172)));
              barEl.style.width = Math.round(pct * 100) + "%";
            }
          });
        }
        lastRoll = lerp(lastRoll, data.roll, 0.2);
        lastPitch = lerp(lastPitch, data.pitch, 0.2);
        drawGauge(lastRoll, lastPitch);
        setValue("status", "Live at " + new Date().toLocaleTimeString());
      } catch (err) {
        setValue("status", "Waiting for data...");
      }
    }

    async function loadConfig() {
      try {
        const response = await fetch("/config");
        const cfg = await response.json();
        Object.keys(cfg).forEach((key) => {
          const input = document.querySelector("[name='" + key + "']");
          if (input) input.value = cfg[key];
        });
      } catch (err) {
        setValue("status", "Config load failed");
      }
    }

    document.getElementById("configForm").addEventListener("submit", async (event) => {
      event.preventDefault();
      const form = event.target;
      const body = new URLSearchParams(new FormData(form));
      try {
        await fetch("/config", {
          method: "POST",
          headers: { "Content-Type": "application/x-www-form-urlencoded" },
          body
        });
        setValue("status", "Config applied");
        loadConfig();
      } catch (err) {
        setValue("status", "Config apply failed");
      }
    });

    loadConfig();
    fetchData();
    setInterval(fetchData, 200);
  </script>
</body>
</html>
)HTML";
