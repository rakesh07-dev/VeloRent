import React, { useEffect, useState } from 'react';
import { apiGet, apiPost } from '../api.js';

export default function BookingsScreen({ vehicles, ledger, onRefresh }) {
  const availableVehicles = vehicles.filter((v) => v.available);
  const [customer, setCustomer] = useState('');
  const [vin, setVin] = useState('');
  const [days, setDays] = useState(3);
  const [tier, setTier] = useState('regular');
  const [quote, setQuote] = useState(null);
  const [feedback, setFeedback] = useState('');
  const [error, setError] = useState('');
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    if (!vin && availableVehicles.length > 0) {
      setVin(availableVehicles[0].vin);
    }
  }, [vehicles]);

  useEffect(() => {
    let active = true;
    async function fetchQuote() {
      if (!vin || !days || Number(days) <= 0) {
        setQuote(null);
        return;
      }
      try {
        const q = await apiGet(`/api/quote?vin=${encodeURIComponent(vin)}&days=${encodeURIComponent(days)}`);
        if (active) setQuote(q.amount);
      } catch (_) {
        if (active) setQuote(null);
      }
    }
    fetchQuote();
    return () => { active = false; };
  }, [vin, days]);

  async function handleCreateBooking(e) {
    e.preventDefault();
    setError('');
    setFeedback('');
    setBusy(true);
    try {
      const created = await apiPost('/api/bookings', {
        customer: customer.trim(),
        vin,
        days: Number(days),
        tier
      });
      setFeedback(
        `Queued booking #${created.id} for ${created.customer} (${created.tier}) — Quoted Rs.${created.amount}`
      );
      setCustomer('');
      await onRefresh();
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  async function handleProcessNext() {
    setError('');
    setFeedback('');
    setBusy(true);
    try {
      const confirmed = await apiPost('/api/bookings/process');
      setFeedback(
        `Confirmed #${confirmed.id} (${confirmed.customer}, ${confirmed.tier}) — Saved to ledger.bin`
      );
      await onRefresh();
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  async function handleUndo() {
    setError('');
    setFeedback('');
    setBusy(true);
    try {
      const rolled = await apiPost('/api/bookings/undo');
      setFeedback(`Rolled back #${rolled.id} (${rolled.customer}) — Vehicle restored & ledger.bin rewritten`);
      await onRefresh();
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  const confirmedList = ledger.confirmed || [];
  const pendingList = ledger.pending || [];

  return (
    <div>
      <div className="page-header">
        <div>
          <h2>Bookings &amp; Ledger Persistence (Min-Heap, FIFO Queue, Undo Stack &amp; RAII)</h2>
          <p>VIP requests enter the min-heap; Regular requests enter the linked-list queue; confirmed bookings persist in ledger.bin</p>
        </div>
        <div className="controls" style={{ marginBottom: 0 }}>
          <button
            className="btn"
            disabled={busy || pendingList.length === 0}
            onClick={handleProcessNext}
          >
            Process Next (VIP First)
          </button>
          <button
            className="btn warn"
            disabled={busy || !ledger.canUndo}
            onClick={handleUndo}
            title={!ledger.canUndo ? 'Undo only works for bookings confirmed in the current backend session' : ''}
          >
            Undo Last Confirmed
          </button>
        </div>
      </div>

      {error && <div className="banner err" role="alert">{error}</div>}
      {feedback && <div className="banner ok" role="status">{feedback}</div>}

      <form className="card" style={{ marginBottom: 20 }} onSubmit={handleCreateBooking}>
        <h3 style={{ marginTop: 0 }}>New Booking Request</h3>
        <div className="controls">
          <div className="field" style={{ flex: 1, minWidth: 180 }}>
            <label htmlFor="bk-cust">Customer Name (max 31 chars in ledger.bin)</label>
            <input
              id="bk-cust"
              placeholder="e.g. Rakesh Sanwal"
              value={customer}
              onChange={(e) => setCustomer(e.target.value)}
            />
          </div>
          <div className="field" style={{ flex: 1, minWidth: 220 }}>
            <label htmlFor="bk-vin">Available Vehicle</label>
            <select id="bk-vin" value={vin} onChange={(e) => setVin(e.target.value)}>
              {vehicles.map((v) => (
                <option key={v.vin} value={v.vin} disabled={!v.available}>
                  {v.vin} — {v.brand} ({v.type}, Rs.{v.baseRate}/d) {!v.available ? '[BOOKED]' : ''}
                </option>
              ))}
            </select>
          </div>
          <div className="field" style={{ width: 110 }}>
            <label htmlFor="bk-days">Days</label>
            <input
              id="bk-days"
              type="number"
              min="1"
              value={days}
              onChange={(e) => setDays(e.target.value)}
            />
          </div>
          <div className="field" style={{ width: 140 }}>
            <label htmlFor="bk-tier">Priority Tier</label>
            <select id="bk-tier" value={tier} onChange={(e) => setTier(e.target.value)}>
              <option value="regular">Regular (FIFO)</option>
              <option value="vip">VIP (Min-Heap)</option>
            </select>
          </div>
          <button type="submit" className="btn" disabled={busy}>
            Submit Request {quote !== null ? `(Rs.${quote})` : ''}
          </button>
        </div>
      </form>

      <div className="grid-2">
        <div className="card">
          <h3 style={{ marginTop: 0 }}>Pending Queue ({pendingList.length})</h3>
          {pendingList.length === 0 ? (
            <p style={{ color: 'var(--muted)' }}>No pending booking requests.</p>
          ) : (
            <div className="table-wrap">
              <table>
                <thead>
                  <tr>
                    <th>ID</th>
                    <th>Customer</th>
                    <th>VIN</th>
                    <th>Days</th>
                    <th>Tier</th>
                    <th>Quoted</th>
                  </tr>
                </thead>
                <tbody>
                  {pendingList.map((b) => (
                    <tr key={b.id}>
                      <td>#{b.id}</td>
                      <td>{b.customer}</td>
                      <td>{b.vin}</td>
                      <td>{b.days}</td>
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
          <h3 style={{ marginTop: 0 }}>Confirmed Ledger (`ledger.bin` — {confirmedList.length})</h3>
          {confirmedList.length === 0 ? (
            <p style={{ color: 'var(--muted)' }}>ledger.bin has 0 confirmed records.</p>
          ) : (
            <div className="table-wrap">
              <table>
                <thead>
                  <tr>
                    <th>ID</th>
                    <th>Customer</th>
                    <th>VIN</th>
                    <th>Days</th>
                    <th>Tier</th>
                    <th>Paid</th>
                  </tr>
                </thead>
                <tbody>
                  {confirmedList.map((b) => (
                    <tr key={b.id}>
                      <td>#{b.id}</td>
                      <td>{b.customer}</td>
                      <td>{b.vin}</td>
                      <td>{b.days}</td>
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
      </div>
    </div>
  );
}
