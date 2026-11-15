// server/index.js -- Express bridge connecting React UI to velorent_api.exe
const express = require('express');
const cors = require('cors');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const readline = require('readline');

const app = express();
const PORT = process.env.PORT || 3001;
const ROOT_DIR = path.resolve(__dirname, '..');

app.use(cors());
app.use(express.json());

function findBackendExe() {
  const names = process.platform === 'win32'
    ? ['velorent_api.exe', 'velorent_api']
    : ['velorent_api', 'velorent_api.exe'];

  for (const name of names) {
    const fullPath = path.join(ROOT_DIR, name);
    if (fs.existsSync(fullPath)) {
      return fullPath;
    }
  }
  return path.join(ROOT_DIR, names[0]);
}

class BackendProcess {
  constructor() {
    this.child = null;
    this.reader = null;
    this.pending = [];
    this.activeReq = null;
    this.running = false;
    this.errorMsg = null;
  }

  start() {
    const exePath = findBackendExe();
    if (!fs.existsSync(exePath)) {
      this.running = false;
      this.errorMsg = `Backend binary missing at ${exePath}`;
      return false;
    }

    try {
      this.child = spawn(exePath, [], {
        cwd: ROOT_DIR,
        stdio: ['pipe', 'pipe', 'pipe']
      });
    } catch (err) {
      this.running = false;
      this.errorMsg = err.message;
      return false;
    }

    this.running = true;
    this.errorMsg = null;

    this.reader = readline.createInterface({ input: this.child.stdout });
    this.reader.on('line', (line) => {
      if (!this.activeReq) {
        return;
      }
      const req = this.activeReq;
      clearTimeout(req.timer);
      this.activeReq = null;

      try {
        req.resolve(JSON.parse(line));
      } catch (err) {
        req.reject(new Error(`Invalid JSON from C++ backend: ${line}`));
      }
      this.next();
    });

    this.child.stderr.on('data', (chunk) => {
      this.errorMsg = chunk.toString().trim();
    });

    this.child.on('exit', (code) => {
      this.running = false;
      this.errorMsg = `velorent_api exited with code ${code}`;
      if (this.activeReq) {
        clearTimeout(this.activeReq.timer);
        this.activeReq.reject(new Error(this.errorMsg));
        this.activeReq = null;
      }
      while (this.pending.length > 0) {
        const item = this.pending.shift();
        item.reject(new Error(this.errorMsg));
      }
    });

    return true;
  }

  checkAlive() {
    if (this.child && this.running && !this.child.killed) {
      return true;
    }
    return this.start();
  }

  send(cmdLine, timeoutMs = 5000) {
    return new Promise((resolve, reject) => {
      if (!this.checkAlive()) {
        reject(new Error(this.errorMsg || 'C++ backend offline'));
        return;
      }
      this.pending.push({ cmdLine, resolve, reject, timeoutMs });
      this.next();
    });
  }

  next() {
    if (this.activeReq || this.pending.length === 0) {
      return;
    }
    if (!this.child || !this.running) {
      return;
    }

    const req = this.pending.shift();
    this.activeReq = req;
    req.timer = setTimeout(() => {
      if (this.activeReq === req) {
        this.activeReq = null;
        req.reject(new Error('C++ backend timed out'));
        try {
          this.child.kill();
        } catch (e) {}
      }
    }, req.timeoutMs);

    try {
      this.child.stdin.write(req.cmdLine + '\n');
    } catch (err) {
      clearTimeout(req.timer);
      this.activeReq = null;
      req.reject(err);
    }
  }

  restart() {
    if (this.child) {
      try {
        this.child.kill();
      } catch (e) {}
      this.child = null;
    }
    this.running = false;
    return this.start();
  }
}

const backend = new BackendProcess();
backend.start();

function wrapArg(val) {
  const str = String(val === undefined || val === null ? '' : val)
    .replace(/[\r\n]/g, ' ')
    .trim();
  return `"${str.replace(/\\/g, '\\\\').replace(/"/g, '\\"')}"`;
}

async function runCommand(res, cmd) {
  try {
    const reply = await backend.send(cmd);
    if (!reply.ok) {
      return res.status(400).json(reply);
    }
    return res.json(reply);
  } catch (err) {
    return res.status(503).json({ ok: false, error: err.message });
  }
}

app.get('/api/health', async (req, res) => {
  await runCommand(res, 'PING');
});

app.post('/api/restart', async (req, res) => {
  if (!backend.restart()) {
    return res.status(503).json({ ok: false, error: backend.errorMsg || 'Restart failed' });
  }
  await runCommand(res, 'PING');
});

app.get('/api/vehicles', async (req, res) => {
  const { min, max } = req.query;
  if (min !== undefined || max !== undefined) {
    if (min === undefined || max === undefined || min === '' || max === '') {
      return res.status(400).json({ ok: false, error: 'Both min and max rate are required' });
    }
    return runCommand(res, `RANGE ${wrapArg(min)} ${wrapArg(max)}`);
  }
  return runCommand(res, 'VEHICLES');
});

app.get('/api/vehicles/:key', async (req, res) => {
  return runCommand(res, `VEHICLE ${wrapArg(req.params.key)}`);
});

app.get('/api/quote', async (req, res) => {
  const { vin, days } = req.query;
  if (!vin || !days) {
    return res.status(400).json({ ok: false, error: 'vin and days are required' });
  }
  return runCommand(res, `QUOTE ${wrapArg(vin)} ${wrapArg(days)}`);
});

app.get('/api/hubs', async (req, res) => {
  return runCommand(res, 'HUBS');
});

app.get('/api/route', async (req, res) => {
  const { from, to, mode = 'oneway' } = req.query;
  if (!from || !to) {
    return res.status(400).json({ ok: false, error: 'Origin and destination hubs are required' });
  }
  return runCommand(res, `ROUTE ${wrapArg(from)} ${wrapArg(to)} ${wrapArg(mode)}`);
});

app.get('/api/network', async (req, res) => {
  const algo = (req.query.algo || 'kruskal').toLowerCase();
  if (algo !== 'kruskal' && algo !== 'prim') {
    return res.status(400).json({ ok: false, error: 'algo must be kruskal or prim' });
  }
  return runCommand(res, algo === 'prim' ? 'MST_PRIM' : 'MST');
});

app.get('/api/reach', async (req, res) => {
  const { hub } = req.query;
  if (!hub) {
    return res.status(400).json({ ok: false, error: 'hub parameter is required' });
  }
  return runCommand(res, `REACH ${wrapArg(hub)}`);
});

app.get('/api/bellman-ford', async (req, res) => {
  const { hub } = req.query;
  if (!hub) {
    return res.status(400).json({ ok: false, error: 'hub parameter is required' });
  }
  return runCommand(res, `BF ${wrapArg(hub)}`);
});

app.get('/api/bookings', async (req, res) => {
  return runCommand(res, 'LEDGER');
});

app.post('/api/bookings', async (req, res) => {
  const { customer, vin, days, tier } = req.body || {};
  if (!customer || !vin || days === undefined || !tier) {
    return res.status(400).json({ ok: false, error: 'customer, vin, days, and tier are required' });
  }
  return runCommand(res, `BOOK ${wrapArg(customer)} ${wrapArg(vin)} ${wrapArg(days)} ${wrapArg(tier)}`);
});

app.post('/api/bookings/process', async (req, res) => {
  return runCommand(res, 'PROCESS');
});

app.post('/api/bookings/undo', async (req, res) => {
  return runCommand(res, 'UNDO');
});

app.get('/api/huffman', async (req, res) => {
  return runCommand(res, 'HUFFMAN');
});

app.get('/api/inspect', async (req, res) => {
  return runCommand(res, 'INSPECT');
});

// Serve the compiled React frontend from web/dist
const distDir = path.join(ROOT_DIR, 'web', 'dist');
if (fs.existsSync(distDir)) {
  app.use(express.static(distDir));
  app.get('*', (req, res, next) => {
    if (req.path.startsWith('/api')) {
      return next();
    }
    res.sendFile(path.join(distDir, 'index.html'));
  });
}

app.listen(PORT, () => {
  console.log(`VeloRent Bridge listening on http://localhost:${PORT}`);
  console.log(`C++ Backend status: ${backend.running ? 'ONLINE' : 'OFFLINE (' + backend.errorMsg + ')'}`);
});
