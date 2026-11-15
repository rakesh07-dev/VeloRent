import React from 'react';

export default function Dashboard({ vehicles, ledger, onNavigate, onProcessNext, busy }) {
  const totalVehicles = vehicles.length;
  const availableVehicles = vehicles.filter((v) => v.available).length;
  const bookedVehicles = totalVehicles - availableVehicles;

  const confirmed = ledger.confirmed || [];
  const pending = ledger.pending || [];
  const totalRevenue = confirmed.reduce((sum, b) => sum + Number(b.amount || 0), 0);

  return (
    <div>
      <div className="page-header">
        <div>
          <h2>Operations Dashboard</h2>
          <p>Live state served directly from C++14 Fleet, BookingManager &amp; ledger.bin</p>
        </div>
        <div className="controls" style={{ marginBottom: 0 }}>
          <button className="btn" onClick={() => onNavigate('bookings')}>+ New Booking</button>
          <button
            className="btn secondary"
            disabled={busy || pending.length === 0}
            onClick={onProcessNext}
          >
            Process Next ({pending.length})
          </button>
          <button className="btn secondary" onClick={() => onNavigate('routes')}>Plan Route</button>
        </div>
      </div>

      <div className="stats-grid">
        <div className="card">
          <div className="stat-label">Total Fleet</div>
          <div className="stat-value">{totalVehicles}</div>
        </div>
        <div className="card">
          <div className="stat-label">Available Vehicles</div>
          <div className="stat-value" style={{ color: 'var(--ok)' }}>{availableVehicles}</div>
        </div>
        <div className="card">
          <div className="stat-label">Booked Vehicles</div>
          <div className="stat-value" style={{ color: 'var(--warn)' }}>{bookedVehicles}</div>
        </div>
        <div className="card">
          <div className="stat-label">Pending Queue (VIP + Reg)</div>
          <div className="stat-value">{pending.length}</div>
        </div>
        <div className="card">
          <div className="stat-label">Confirmed (ledger.bin)</div>
          <div className="stat-value">{confirmed.length}</div>
        </div>
        <div className="card">
          <div className="stat-label">Ledger Revenue</div>
          <div className="stat-value" style={{ color: 'var(--accent)' }}>
            Rs.{totalRevenue.toLocaleString('en-IN')}
          </div>
        </div>
      </div>

      <div className="grid-2">
        <div className="card">
          <h3 style={{ marginTop: 0 }}>Confirmed Bookings (ledger.bin)</h3>
          {confirmed.length === 0 ? (
            <p style={{ color: 'var(--muted)' }}>No confirmed bookings persisted in ledger.bin yet.</p>
          ) : (
            <div className="table-wrap">
              <table>
                <thead>
                  <tr>
                    <th>ID</th>
                    <th>Customer</th>
                    <th>VIN</th>
                    <th>Tier</th>
                    <th>Amount</th>
                  </tr>
                </thead>
                <tbody>
                  {[...confirmed].reverse().slice(0, 6).map((b) => (
                    <tr key={b.id}>
                      <td>#{b.id}</td>
                      <td>{b.customer}</td>
                      <td>{b.vin}</td>
                      <td>
                        <span className={`pill ${b.tier === 'VIP' ? 'warn' : 'accent'}`}>{b.tier}</span>
                      </td>
                      <td>Rs.{b.amount}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </div>

        <div className="card">
          <h3 style={{ marginTop: 0 }}>Pending Requests (VIP Heap &amp; Regular Queue)</h3>
          {pending.length === 0 ? (
            <p style={{ color: 'var(--muted)' }}>Pending queue is empty.</p>
          ) : (
            <div className="table-wrap">
              <table>
                <thead>
                  <tr>
                    <th>ID</th>
                    <th>Customer</th>
                    <th>VIN</th>
                    <th>Days</th>
                    <th>Priority</th>
                  </tr>
                </thead>
                <tbody>
                  {pending.map((b) => (
                    <tr key={b.id}>
                      <td>#{b.id}</td>
                      <td>{b.customer}</td>
                      <td>{b.vin}</td>
                      <td>{b.days}</td>
                      <td>
                        <span className={`pill ${b.tier === 'VIP' ? 'warn' : 'accent'}`}>{b.tier}</span>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
