import React, { useEffect, useState } from 'react';
import { apiGet } from '../api.js';
import HubMap from './HubMap.jsx';

export default function AlgorithmsScreen({ hubData }) {
  const hubs = hubData.hubs || [];
  const routes = hubData.routes || [];

  const [selectedHub, setSelectedHub] = useState('Dehradun');
  const [kruskal, setKruskal] = useState(null);
  const [prim, setPrim] = useState(null);
  const [reach, setReach] = useState(null);
  const [bf, setBf] = useState(null);
  const [huffman, setHuffman] = useState(null);
  const [error, setError] = useState('');

  async function loadGraphDemos(hubName) {
    setError('');
    try {
      const [kData, pData, rData, bfData, huffData] = await Promise.all([
        apiGet('/api/network?algo=kruskal'),
        apiGet('/api/network?algo=prim'),
        apiGet(`/api/reach?hub=${encodeURIComponent(hubName)}`),
        apiGet(`/api/bellman-ford?hub=${encodeURIComponent(hubName)}`),
        apiGet('/api/huffman')
      ]);
      setKruskal(kData);
      setPrim(pData);
      setReach(rData);
      setBf(bfData);
      setHuffman(huffData);
    } catch (err) {
      setError(err.message);
    }
  }

  useEffect(() => {
    loadGraphDemos(selectedHub);
  }, [selectedHub]);

  return (
    <div>
      <div className="page-header">
        <div>
          <h2>DSA &amp; Viva Demonstrations (MST, BFS/DFS, Bellman-Ford &amp; Huffman)</h2>
          <p>Live execution from C++ HubGraph &amp; RateTree::HuffmanCoder</p>
        </div>
        <div className="field">
          <label htmlFor="algo-hub">Traversal / Bellman-Ford Start Hub</label>
          <select
            id="algo-hub"
            value={selectedHub}
            onChange={(e) => setSelectedHub(e.target.value)}
          >
            {hubs.map((h) => (
              <option key={h.id} value={h.name}>{h.name}</option>
            ))}
          </select>
        </div>
      </div>

      {error && <div className="banner err" role="alert">{error}</div>}

      <div className="grid-2">
        <div className="card">
          <h3 style={{ marginTop: 0 }}>Minimum Spanning Tree: Kruskal vs Prim</h3>
          {kruskal && prim && (
            <>
              <div className="stats-grid">
                <div className="card" style={{ background: 'var(--surface2)' }}>
                  <div className="stat-label">Kruskal + Union-Find</div>
                  <div className="stat-value" style={{ color: 'var(--ok)' }}>{kruskal.totalCost} km</div>
                </div>
                <div className="card" style={{ background: 'var(--surface2)' }}>
                  <div className="stat-label">Prim's Algorithm</div>
                  <div className="stat-value" style={{ color: 'var(--ok)' }}>{prim.totalCost} km</div>
                </div>
              </div>
              <HubMap hubs={hubs} routes={routes} mstEdges={kruskal.edges} />
            </>
          )}
        </div>

        <div className="card">
          <h3 style={{ marginTop: 0 }}>BFS &amp; DFS Reachability from {selectedHub}</h3>
          {reach && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: 12 }}>
              <div className="card" style={{ background: 'var(--surface2)' }}>
                <div className="stat-label">BFS Order (Queue — Fewest Hops First)</div>
                <div style={{ marginTop: 6, fontSize: '0.9rem' }}>{reach.bfs.join(' → ')}</div>
              </div>
              <div className="card" style={{ background: 'var(--surface2)' }}>
                <div className="stat-label">DFS Order (Recursion — Deepest First)</div>
                <div style={{ marginTop: 6, fontSize: '0.9rem' }}>{reach.dfs.join(' → ')}</div>
              </div>
            </div>
          )}

          {bf && (
            <div style={{ marginTop: 16 }}>
              <h4>Bellman-Ford Distances from {selectedHub}</h4>
              <div className="table-wrap" style={{ maxHeight: 220, overflowY: 'auto' }}>
                <table>
                  <thead>
                    <tr>
                      <th>Destination Hub</th>
                      <th>Shortest Distance</th>
                    </tr>
                  </thead>
                  <tbody>
                    {bf.distances.map((row) => (
                      <tr key={row.hub}>
                        <td>{row.hub}</td>
                        <td>{row.reachable ? `${row.distanceKm} km` : 'Unreachable'}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>
          )}
        </div>
      </div>

      {huffman && (
        <div className="card">
          <h3 style={{ marginTop: 0 }}>Huffman Receipt Compression (`RateTree.h`)</h3>
          <div className="stats-grid">
            <div className="card" style={{ background: 'var(--surface2)' }}>
              <div className="stat-label">Original Size (8-bit ASCII)</div>
              <div className="stat-value">{huffman.originalBits} bits</div>
            </div>
            <div className="card" style={{ background: 'var(--surface2)' }}>
              <div className="stat-label">Huffman Compressed</div>
              <div className="stat-value" style={{ color: 'var(--ok)' }}>{huffman.compressedBits} bits</div>
            </div>
            <div className="card" style={{ background: 'var(--surface2)' }}>
              <div className="stat-label">Ratio &amp; Decode Check</div>
              <div className="stat-value">
                {huffman.ratioPercent}% ({huffman.roundTripMatch ? 'Verified' : 'Mismatch'})
              </div>
            </div>
          </div>
          <div className="grid-2" style={{ marginBottom: 0 }}>
            <div>
              <div className="stat-label" style={{ marginBottom: 6 }}>Receipt Input Text</div>
              <div className="mono-box">{huffman.sampleText}</div>
            </div>
            <div>
              <div className="stat-label" style={{ marginBottom: 6 }}>Encoded Bitstream Preview</div>
              <div className="mono-box">{huffman.encodedPreview}...</div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
