import React, { useEffect, useState } from 'react';
import { apiGet, apiPost } from './api.js';
import Dashboard from './components/Dashboard.jsx';
import FleetScreen from './components/FleetScreen.jsx';
import BookingsScreen from './components/BookingsScreen.jsx';
import RoutePlannerScreen from './components/RoutePlannerScreen.jsx';
import AlgorithmsScreen from './components/AlgorithmsScreen.jsx';

const NAV_ITEMS = [
  { id: 'dashboard', label: 'Dashboard' },
  { id: 'fleet', label: 'Fleet Management' },
  { id: 'bookings', label: 'Bookings & Ledger' },
  { id: 'routes', label: 'Route Planner' },
  { id: 'algorithms', label: 'DSA & Algorithms' }
];

export default function App() {
  const [tab, setTab] = useState(() => localStorage.getItem('velorent_tab') || 'dashboard');
  const [online, setOnline] = useState(false);
  const [backendError, setBackendError] = useState('');
  const [vehicles, setVehicles] = useState([]);
  const [ledger, setLedger] = useState({ confirmed: [], pending: [], canUndo: false });
  const [hubData, setHubData] = useState({ hubs: [], routes: [] });
  const [busy, setBusy] = useState(false);

  function selectTab(nextTab) {
    setTab(nextTab);
    localStorage.setItem('velorent_tab', nextTab);
  }

  async function refreshAll() {
    try {
      await apiGet('/api/health');
      const [vList, bState, hState] = await Promise.all([
        apiGet('/api/vehicles'),
        apiGet('/api/bookings'),
        apiGet('/api/hubs')
      ]);
      setOnline(true);
      setBackendError('');
      setVehicles(vList);
      setLedger(bState);
      setHubData(hState);
    } catch (err) {
      setOnline(false);
      setBackendError(err.message);
    }
  }

  useEffect(() => {
    refreshAll();
  }, []);

  async function handleQuickProcess() {
    setBusy(true);
    try {
      await apiPost('/api/bookings/process');
      await refreshAll();
    } catch (err) {
      setBackendError(err.message);
    } finally {
      setBusy(false);
    }
  }

  return (
    <div className="layout">
      <aside className="sidebar">
        <div className="brand">
          <h1>VELORENT</h1>
          <p>Team The Hamsters · GEHU</p>
        </div>

        <div className="status-badge" role="status">
          <span className={`dot ${online ? 'online' : 'offline'}`} />
          <span>{online ? 'Connected to C++ backend' : 'Backend offline'}</span>
        </div>

        <nav className="nav-list" aria-label="Main navigation">
          {NAV_ITEMS.map((item) => (
            <button
              key={item.id}
              className={`nav-btn ${tab === item.id ? 'active' : ''}`}
              onClick={() => selectTab(item.id)}
            >
              {item.label}
            </button>
          ))}
        </nav>

        <div className="sidebar-footer">
          <div>C++14 Single Source of Truth</div>
          <div style={{ marginTop: 4 }}>TCS302 DSA + TCS307 OOP</div>
        </div>
      </aside>

      <main className="main">
        {!online && (
          <div className="banner err" role="alert">
            <strong>Backend offline:</strong> {backendError || 'Unable to reach C++ bridge.'}{' '}
            Start <code>velorent_api</code> + Node bridge (<code>npm start</code> in <code>server/</code>) and{' '}
            <button className="btn secondary" style={{ marginLeft: 8, padding: '4px 10px' }} onClick={refreshAll}>
              Retry
            </button>
          </div>
        )}

        {tab === 'dashboard' && (
          <Dashboard
            vehicles={vehicles}
            ledger={ledger}
            onNavigate={selectTab}
            onProcessNext={handleQuickProcess}
            busy={busy}
          />
        )}
        {tab === 'fleet' && <FleetScreen vehicles={vehicles} onRefresh={refreshAll} />}
        {tab === 'bookings' && (
          <BookingsScreen vehicles={vehicles} ledger={ledger} onRefresh={refreshAll} />
        )}
        {tab === 'routes' && <RoutePlannerScreen hubData={hubData} />}
        {tab === 'algorithms' && <AlgorithmsScreen hubData={hubData} />}
      </main>
    </div>
  );
}
