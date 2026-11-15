import React, { useEffect, useState } from 'react';
import { apiGet } from '../api.js';
import HubMap from './HubMap.jsx';

export default function RoutePlannerScreen({ hubData }) {
  const hubs = hubData.hubs || [];
  const routes = hubData.routes || [];

  const [from, setFrom] = useState('Dehradun');
  const [to, setTo] = useState('Delhi');
  const [mode, setMode] = useState('oneway');
  const [result, setResult] = useState(null);
  const [error, setError] = useState('');
  const [busy, setBusy] = useState(false);

  async function calculateRoute(e) {
    if (e) e.preventDefault();
    setError('');
    setBusy(true);
    try {
      const data = await apiGet(
        `/api/route?from=${encodeURIComponent(from)}&to=${encodeURIComponent(to)}&mode=${encodeURIComponent(mode)}`
      );
      setResult(data);
    } catch (err) {
      setError(err.message);
      setResult(null);
    } finally {
      setBusy(false);
    }
  }

  useEffect(() => {
    if (hubs.length > 0) calculateRoute();
  }, [hubs.length]);

  return (
    <div>
      <div className="page-header">
        <div>
          <h2>Route Planner &amp; Drop-Off Strategy (Dijkstra + Strategy Pattern)</h2>
          <p>Computed directly by C++ HubGraph::dijkstra and DropOffCalculator</p>
        </div>
      </div>

      {error && <div className="banner err" role="alert">{error}</div>}

      <div className="grid-2">
        <div className="card">
          <form onSubmit={calculateRoute}>
            <div className="controls">
              <div className="field" style={{ flex: 1 }}>
                <label htmlFor="rt-from">Pickup Hub</label>
                <select id="rt-from" value={from} onChange={(e) => setFrom(e.target.value)}>
                  {hubs.map((h) => (
                    <option key={h.id} value={h.name}>{h.name}</option>
                  ))}
                </select>
              </div>
              <div className="field" style={{ flex: 1 }}>
                <label htmlFor="rt-to">Drop-off Hub</label>
                <select id="rt-to" value={to} onChange={(e) => setTo(e.target.value)}>
                  {hubs.map((h) => (
                    <option key={h.id} value={h.name}>{h.name}</option>
                  ))}
                </select>
              </div>
              <div className="field" style={{ flex: 1 }}>
                <label htmlFor="rt-mode">Pricing Strategy</label>
                <select id="rt-mode" value={mode} onChange={(e) => setMode(e.target.value)}>
                  <option value="same">Same-hub return (Rs.0)</option>
                  <option value="oneway">One-way (Rs.200 + Rs.12/km)</option>
                  <option value="peak">Peak season (One-way × 1.25)</option>
                </select>
              </div>
              <button type="submit" className="btn" disabled={busy}>Calculate Route</button>
            </div>
          </form>

          {result && (
            <div style={{ marginTop: 16 }}>
              <div className="stats-grid">
                <div className="card" style={{ background: 'var(--surface2)' }}>
                  <div className="stat-label">Shortest Distance</div>
                  <div className="stat-value">{result.distanceKm} km</div>
                </div>
                <div className="card" style={{ background: 'var(--surface2)' }}>
                  <div className="stat-label">{result.strategy}</div>
                  <div className="stat-value" style={{ color: 'var(--ok)' }}>Rs.{result.charge}</div>
                </div>
              </div>
              <div className="card" style={{ background: 'var(--surface2)' }}>
                <div className="stat-label" style={{ marginBottom: 6 }}>Dijkstra Shortest Path</div>
                <div style={{ fontWeight: 600, fontSize: '1.05rem' }}>
                  {result.path.join(' → ')}
                </div>
              </div>
            </div>
          )}
        </div>

        <div className="card">
          <h3 style={{ marginTop: 0 }}>14-Hub Network Topology (Dashed = One-Way)</h3>
          <HubMap
            hubs={hubs}
            routes={routes}
            activePath={result ? result.path : []}
          />
        </div>
      </div>
    </div>
  );
}
