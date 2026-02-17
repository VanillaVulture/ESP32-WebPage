const mockAps = [
  { ssid: 'Coffee_WiFi_Free', rssi: -81, channel: 1, security: 'OPEN' },
  { ssid: 'HomeNet-5G', rssi: -47, channel: 44, security: 'WPA2/WPA3' },
  { ssid: 'LegacyOffice', rssi: -66, channel: 6, security: 'WEP' },
  { ssid: 'IoT-Lab', rssi: -59, channel: 11, security: 'WPA2' },
  { ssid: 'GuestLobby', rssi: -73, channel: 149, security: 'OPEN' },
  { ssid: 'MobileHotspot', rssi: -54, channel: 36, security: 'WPA2' },
];

const state = { aps: [] };
const tableBody = document.getElementById('apTableBody');
const logOutput = document.getElementById('logOutput');

const securityRisk = (security) => {
  if (security === 'OPEN') return { label: 'High - No Password', cls: 'high' };
  if (security === 'WEP') return { label: 'Medium - Deprecated', cls: 'med' };
  if (security === 'WPA2') return { label: 'Low - Acceptable', cls: 'low' };
  return { label: 'Low - Strong', cls: 'low' };
};

const signalClass = (rssi) => {
  if (rssi >= -55) return 'signal-good';
  if (rssi >= -72) return 'signal-mid';
  return 'signal-bad';
};

const log = (msg) => {
  const timestamp = new Date().toLocaleTimeString();
  logOutput.textContent = `[${timestamp}] ${msg}\n${logOutput.textContent}`;
};

const updateStats = () => {
  const total = state.aps.length;
  const open = state.aps.filter((ap) => ap.security === 'OPEN').length;
  const weak = state.aps.filter((ap) => ['OPEN', 'WEP'].includes(ap.security)).length;
  const strong = total - weak;

  document.getElementById('totalAps').textContent = total;
  document.getElementById('openAps').textContent = open;
  document.getElementById('weakAps').textContent = weak;
  document.getElementById('strongAps').textContent = strong;
};

const renderTable = () => {
  tableBody.innerHTML = '';

  state.aps.forEach((ap) => {
    const risk = securityRisk(ap.security);
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td>${ap.ssid}</td>
      <td class="${signalClass(ap.rssi)}">${ap.rssi} dBm</td>
      <td>${ap.channel}</td>
      <td>${ap.security}</td>
      <td><span class="badge badge-risk-${risk.cls}">${risk.label}</span></td>
      <td><button class="btn deauth-btn" data-ssid="${ap.ssid}">Deauth</button></td>
    `;
    tableBody.appendChild(tr);
  });

  tableBody.querySelectorAll('.deauth-btn').forEach((button) => {
    button.addEventListener('click', () => {
      const { ssid } = button.dataset;
      log(`[SIMULATED] Deauth requested for ${ssid}. No packets sent (stub only).`);
      console.log(`[SIMULATED] Deauth requested for ${ssid}`);
    });
  });
};

const scanAps = () => {
  state.aps = [...mockAps].sort(() => Math.random() - 0.5);
  renderTable();
  updateStats();
  log(`Scan complete. Detected ${state.aps.length} APs.`);
};

document.getElementById('scanBtn').addEventListener('click', scanAps);

document.getElementById('sortSignalBtn').addEventListener('click', () => {
  state.aps.sort((a, b) => b.rssi - a.rssi);
  renderTable();
  log('Sorted AP table by strongest signal.');
});

document.getElementById('channelCheckBtn').addEventListener('click', () => {
  const overcrowded = [1, 6, 11].map((ch) => ({
    ch,
    count: state.aps.filter((ap) => ap.channel === ch).length,
  }));
  const busiest = overcrowded.sort((a, b) => b.count - a.count)[0];
  log(`2.4 GHz check: channel ${busiest.ch} has ${busiest.count} nearby AP(s).`);
});

document.getElementById('copySummaryBtn').addEventListener('click', async () => {
  const vulnerable = state.aps.filter((ap) => ['OPEN', 'WEP'].includes(ap.security));
  const summary = vulnerable.length
    ? vulnerable.map((ap) => `${ap.ssid} (${ap.security})`).join(', ')
    : 'No obvious weak APs found in current scan.';

  try {
    await navigator.clipboard.writeText(summary);
    log('Vulnerability summary copied to clipboard.');
  } catch {
    log(`Clipboard not available. Summary: ${summary}`);
  }
});

document.getElementById('exportBtn').addEventListener('click', () => {
  const payload = {
    generatedAt: new Date().toISOString(),
    device: 'ESP32-Core-Mock',
    aps: state.aps,
  };

  const blob = new Blob([JSON.stringify(payload, null, 2)], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = 'esp32-ap-scan.json';
  a.click();
  URL.revokeObjectURL(url);
  log('Exported current AP scan as JSON.');
});

scanAps();
