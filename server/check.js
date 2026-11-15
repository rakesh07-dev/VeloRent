// server/check.js -- end-to-end checks for velorent_api.exe
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const readline = require('readline');

const ROOT_DIR = path.resolve(__dirname, '..');
const exePath = process.platform === 'win32'
  ? path.join(ROOT_DIR, 'velorent_api.exe')
  : path.join(ROOT_DIR, 'velorent_api');

if (!fs.existsSync(exePath)) {
  console.error('ERROR: velorent_api not found. Compile src/api_main.cpp first.');
  process.exit(1);
}

const ledgerFile = path.join(ROOT_DIR, 'ledger.bin');
if (fs.existsSync(ledgerFile)) {
  fs.unlinkSync(ledgerFile);
}

function launchBackend() {
  const child = spawn(exePath, [], { cwd: ROOT_DIR, stdio: ['pipe', 'pipe', 'pipe'] });
  const reader = readline.createInterface({ input: child.stdout });
  const queue = [];

  reader.on('line', (line) => {
    const resolve = queue.shift();
    if (resolve) {
      resolve(JSON.parse(line));
    }
  });

  return {
    send(cmd) {
      return new Promise((resolve) => {
        queue.push(resolve);
        child.stdin.write(cmd + '\n');
      });
    },
    stop() {
      child.kill();
    }
  };
}

async function runChecks() {
  let passed = 0;

  function verify(label, ok) {
    if (!ok) {
      console.error('[FAIL]', label);
      process.exit(1);
    }
    passed++;
    console.log('[PASS]', label);
  }

  let app = launchBackend();

  const ping = await app.send('PING');
  verify('PING reports 30 vehicles and 14 hubs',
    ping.ok && ping.data.vehicles === 30 && ping.data.hubs === 14);

  const r1 = await app.send('ROUTE Dehradun Delhi oneway');
  verify('Dehradun -> Delhi one-way = 261 km, Rs.3332, path Dehradun->Haridwar->Roorkee->Delhi',
    r1.ok && r1.data.distanceKm === 261 && r1.data.charge === 3332 &&
    r1.data.path.join(',') === 'Dehradun,Haridwar,Roorkee,Delhi');

  const r2 = await app.send('ROUTE Dehradun Jaipur oneway');
  const r2Peak = await app.send('ROUTE Dehradun Jaipur peak');
  verify('Dehradun -> Jaipur = 541 km, one-way Rs.6692, peak Rs.8365',
    r2.ok && r2.data.distanceKm === 541 && r2.data.charge === 6692 && r2Peak.data.charge === 8365);

  const r3 = await app.send('ROUTE Nainital Jaipur peak');
  verify('Nainital -> Jaipur peak = 590 km, Rs.9100',
    r3.ok && r3.data.distanceKm === 590 && r3.data.charge === 9100);

  const cd = await app.send('ROUTE Chandigarh Delhi oneway');
  const dc = await app.send('ROUTE Delhi Chandigarh oneway');
  verify('Directed graph: Chandigarh->Delhi (250) vs Delhi->Chandigarh (245)',
    cd.data.distanceKm === 250 && dc.data.distanceKm === 245);

  const sameHub = await app.send('ROUTE Dehradun Dehradun same');
  verify('Same hub charge = Rs.0',
    sameHub.ok && sameHub.data.distanceKm === 0 && sameHub.data.charge === 0);

  const kruskal = await app.send('MST');
  const prim = await app.send('MST_PRIM');
  verify('Kruskal and Prim MST both equal 1148 km',
    kruskal.data.totalCost === 1148 && prim.data.totalCost === 1148);

  const quote = await app.send('QUOTE VIN1003 4');
  verify('Toyota Innova (VIN1003) 4 days = Rs.12800',
    quote.ok && quote.data.amount === 12800);

  // Test VIP min-heap priority and FIFO queue ordering
  await app.send('BOOK "Regular User" VIN1001 2 regular');
  await app.send('BOOK "VIP Guest" VIN1003 4 vip');

  const p1 = await app.send('PROCESS');
  verify('VIP booking confirmed before earlier regular booking',
    p1.ok && p1.data.vin === 'VIN1003' && p1.data.tier === 'VIP');

  const p2 = await app.send('PROCESS');
  verify('Regular booking confirmed second',
    p2.ok && p2.data.vin === 'VIN1001');

  const undo = await app.send('UNDO');
  verify('Undo rolls back latest confirmed booking (VIN1001)',
    undo.ok && undo.data.vin === 'VIN1001');

  // Restart process to test ledger.bin recovery
  app.stop();
  app = launchBackend();

  const after = await app.send('LEDGER');
  verify('After restart: 1 booking recovered from ledger.bin and session undo stack is empty',
    after.ok && after.data.confirmed.length === 1 &&
    after.data.confirmed[0].vin === 'VIN1003' && after.data.canUndo === false);

  const v1003 = await app.send('VEHICLE VIN1003');
  verify('After restart: VIN1003 is marked Booked',
    v1003.ok && v1003.data.status === 'Booked');

  const badHub = await app.send('ROUTE Mars Delhi oneway');
  const badVin = await app.send('VEHICLE NOPE99');
  const badDays = await app.send('BOOK "Test" VIN1002 0 vip');
  const badRange = await app.send('RANGE 5000 1000');
  const badCmd = await app.send('INVALID_CMD');
  verify('Invalid inputs return structured errors without crashing',
    !badHub.ok && !badVin.ok && !badDays.ok && !badRange.ok && !badCmd.ok);

  app.stop();
  if (fs.existsSync(ledgerFile)) {
    fs.unlinkSync(ledgerFile);
  }
  console.log(`\nAll ${passed} acceptance checks passed.`);
}

runChecks();
