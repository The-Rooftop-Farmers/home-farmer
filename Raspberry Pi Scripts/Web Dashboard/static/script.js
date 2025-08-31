const fmt = ts => new Date(ts).toLocaleTimeString();
const tCtx = document.getElementById('tChart').getContext('2d');
const hCtx = document.getElementById('hChart').getContext('2d');

const tData = {labels: [], datasets: [{label: '°C', data: []}]};
const hData = {labels: [], datasets: [{label: '%', data: []}]};

const baseOpts = {
  responsive:true,
  plugins:{legend:{display:false}},
  scales:{x:{ticks:{color:'#94a3b8'}}, y:{ticks:{color:'#94a3b8'}}}
};
const tChart = new Chart(tCtx, {type:'line', data:tData, options:baseOpts});
const hChart = new Chart(hCtx, {type:'line', data:hData, options:baseOpts});

function pushPoint(chart, labels, dataArr, label, val) {
  labels.push(label);
  dataArr.push(val);
  if (labels.length > 60) { labels.shift(); dataArr.shift(); }
  chart.update('none');
}

async function pull() {
  const r = await fetch('/sensors');
  const s = await r.json();

  document.getElementById('tNow').textContent =
    s.temperature !== null ? (s.temperature.toFixed ? s.temperature.toFixed(1) : s.temperature) + ' °C' : 'N/A';
  document.getElementById('hNow').textContent =
    s.humidity !== null ? (s.humidity.toFixed ? s.humidity.toFixed(1) : s.humidity) + ' %' : 'N/A';
  document.getElementById('sNow').textContent =
    s.soil_moisture !== null ? s.soil_moisture + ' %' : 'N/A';
  document.getElementById('timeNow').textContent = fmt(s.timestamp);

  const label = new Date(s.timestamp).toLocaleTimeString();
  if (s.temperature !== null && !isNaN(s.temperature))
    pushPoint(tChart, tData.labels, tData.datasets[0].data, label, s.temperature);
  if (s.humidity !== null && !isNaN(s.humidity))
    pushPoint(hChart, hData.labels, hData.datasets[0].data, label, s.humidity);
}

// warm start with history
(async () => {
  const r = await fetch('/sensors/history');
  const hist = await r.json();
  hist.forEach(s => {
    const label = new Date(s.timestamp).toLocaleTimeString();
    if (s.temperature !== null) pushPoint(tChart, tData.labels, tData.datasets[0].data, label, s.temperature);
    if (s.humidity !== null)    pushPoint(hChart, hData.labels, hData.datasets[0].data, label, s.humidity);
  });
  await pull();
  setInterval(pull, 2000);
})();
