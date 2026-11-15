import React, { useState } from 'react';
import { apiGet } from '../api.js';

export default function FleetScreen({ vehicles, onRefresh }) {
  const [typeFilter, setTypeFilter] = useState('ALL');
  const [statusFilter, setStatusFilter] = useState('ALL');
  const [lookupKey, setLookupKey] = useState('');
  const [lookupResult, setLookupResult] = useState(null);
  const [minRate, setMinRate] = useState('');
  const [maxRate, setMaxRate] = useState('');
  const [avlVehicles, setAvlVehicles] = useState(null);
  const [avlMeta, setAvlMeta] = useState(null);
  const [inspectReport, setInspectReport] = useState('');
  const [error, setError] = useState('');

  async function handleHashLookup(e) {
    e.preventDefault();
    setError('');
    setLookupResult(null);
    if (!lookupKey.trim()) return;
    try {
      const data = await apiGet(`/api/vehicles/${encodeURIComponent(lookupKey.trim())}`);
      setLookupResult(data);
    } catch (err) {
      setError(err.message);
    }
  }

  async function handleAvlRange(e) {
    e.preventDefault();
    setError('');
    try {
      const data = await apiGet(
        `/api/vehicles?min=${encodeURIComponent(minRate)}&max=${encodeURIComponent(maxRate)}`
      );
      setAvlVehicles(data.vehicles);
      setAvlMeta({ height: data.avlHeight, size: data.avlSize });
    } catch (err) {
      setError(err.message);
    }
  }

  function clearAvlFilter() {
    setAvlVehicles(null);
    setAvlMeta(null);
    setMinRate('');
    setMaxRate('');
    setError('');
  }

  async function runMaintenanceInspect() {
    setError('');
    try {
      const data = await apiGet('/api/inspect');
      setInspectReport(data.report);
    } catch (err) {
      setError(err.message);
    }
  }

  const baseList = avlVehicles !== null ? avlVehicles : vehicles;
  const displayed = baseList.filter((v) => {
    if (typeFilter !== 'ALL' && v.type !== typeFilter) return false;
    if (statusFilter === 'AVAILABLE' && !v.available) return false;
    if (statusFilter === 'BOOKED' && v.available) return false;
    return true;
  });

  return (
    <div>
      <div className="page-header">
        <div>
          <h2>Fleet Management (Merge Sort, Hash Table &amp; AVL Index)</h2>
          <p>Sorted by daily rate via C++ recursive merge sort; indexed by djb2 hash table and AVL tree</p>
        </div>
        <div className="controls" style={{ marginBottom: 0 }}>
          <button className="btn secondary" onClick={runMaintenanceInspect}>
            Run Polymorphic Inspection
          </button>
          <button className="btn secondary" onClick={onRefresh}>Refresh Fleet</button>
        </div>
      </div>

      {error && <div className="banner err" role="alert">{error}</div>}

      <div className="grid-2">
        <form className="card" onSubmit={handleHashLookup}>
          <h3 style={{ marginTop: 0 }}>O(1) Hash Table Lookup (VIN or Plate)</h3>
          <div className="controls">
            <div className="field" style={{ flex: 1 }}>
              <label htmlFor="hash-key">Exact VIN or Licence Plate</label>
              <input
                id="hash-key"
                placeholder="e.g. VIN1003 or DL01XY0007"
                value={lookupKey}
                onChange={(e) => setLookupKey(e.target.value)}
              />
            </div>
            <button type="submit" className="btn">Lookup</button>
          </div>
          {lookupResult && (
            <div className="banner ok" style={{ marginBottom: 0 }}>
              Found <strong>{lookupResult.brand}</strong> ({lookupResult.type}) · {lookupResult.vin} /{' '}
              {lookupResult.plate} · Rs.{lookupResult.baseRate}/day · Status: {lookupResult.status}
            </div>
          )}
        </form>

        <form className="card" onSubmit={handleAvlRange}>
          <h3 style={{ marginTop: 0 }}>O(log n + k) AVL Price Range Search</h3>
          <div className="controls">
            <div className="field">
              <label htmlFor="avl-min">Min Rate (Rs.)</label>
              <input
                id="avl-min"
                type="number"
                placeholder="1000"
                value={minRate}
                onChange={(e) => setMinRate(e.target.value)}
              />
            </div>
            <div className="field">
              <label htmlFor="avl-max">Max Rate (Rs.)</label>
              <input
                id="avl-max"
                type="number"
                placeholder="3000"
                value={maxRate}
                onChange={(e) => setMaxRate(e.target.value)}
              />
            </div>
            <button type="submit" className="btn">Search Range</button>
            {avlVehicles !== null && (
              <button type="button" className="btn secondary" onClick={clearAvlFilter}>Reset</button>
            )}
          </div>
          {avlMeta && (
            <div style={{ fontSize: '0.82rem', color: 'var(--muted)' }}>
              AVL Tree Height: {avlMeta.height} (for {avlMeta.size} nodes) · Showing {avlVehicles.length} matches
            </div>
          )}
        </form>
      </div>

      {inspectReport && (
        <div className="card" style={{ marginBottom: 16 }}>
          <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
            <h3 style={{ margin: '0 0 8px' }}>Polymorphic Maintenance Inspection (`Vehicle::inspectMaintenance`)</h3>
            <button className="btn secondary" onClick={() => setInspectReport('')}>Close</button>
          </div>
          <div className="mono-box">{inspectReport}</div>
        </div>
      )}

      <div className="card">
        <div className="controls">
          <div className="field">
            <label htmlFor="filter-type">Vehicle Class</label>
            <select id="filter-type" value={typeFilter} onChange={(e) => setTypeFilter(e.target.value)}>
              <option value="ALL">All Types ({baseList.length})</option>
              <option value="Car">Car</option>
              <option value="Electric">Electric</option>
              <option value="Bike">Bike</option>
              <option value="LuxuryBus">LuxuryBus</option>
            </select>
          </div>
          <div className="field">
            <label htmlFor="filter-status">Availability</label>
            <select id="filter-status" value={statusFilter} onChange={(e) => setStatusFilter(e.target.value)}>
              <option value="ALL">All Statuses</option>
              <option value="AVAILABLE">Available Only</option>
              <option value="BOOKED">Booked Only</option>
            </select>
          </div>
        </div>

        <div className="table-wrap">
          <table>
            <thead>
              <tr>
                <th>VIN</th>
                <th>Plate</th>
                <th>Type</th>
                <th>Model</th>
                <th>Base Rate</th>
                <th>1-Day Tariff</th>
                <th>7-Day Tariff</th>
                <th>Status</th>
              </tr>
            </thead>
            <tbody>
              {displayed.length === 0 ? (
                <tr>
                  <td colSpan="8" style={{ color: 'var(--muted)' }}>No vehicles match the active filters.</td>
                </tr>
              ) : (
                displayed.map((v) => (
                  <tr key={v.vin}>
                    <td><strong>{v.vin}</strong></td>
                    <td>{v.plate}</td>
                    <td>{v.type}</td>
                    <td>{v.brand}</td>
                    <td>Rs.{v.baseRate}/day</td>
                    <td>Rs.{v.tariff1Day}</td>
                    <td>Rs.{v.tariff7Days}</td>
                    <td>
                      <span className={`pill ${v.available ? 'ok' : 'err'}`}>{v.status}</span>
                    </td>
                  </tr>
                ))
              )}
            </tbody>
          </table>
        </div>
      </div>
    </div>
  );
}
