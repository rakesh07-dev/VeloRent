import React from 'react';

// Schematic coordinates for the 14 hubs in HubGraph
const HUB_COORDS = {
  Chandigarh: { x: 110, y: 75 },
  Ambala:     { x: 145, y: 125 },
  Dehradun:   { x: 255, y: 65 },
  Mussoorie:  { x: 255, y: 24 },
  Rishikesh:  { x: 315, y: 80 },
  Haridwar:   { x: 295, y: 120 },
  Roorkee:    { x: 245, y: 145 },
  Saharanpur: { x: 195, y: 155 },
  Meerut:     { x: 225, y: 205 },
  Delhi:      { x: 195, y: 255 },
  Haldwani:   { x: 395, y: 155 },
  Nainital:   { x: 425, y: 115 },
  Agra:       { x: 255, y: 315 },
  Jaipur:     { x: 115, y: 325 }
};

export default function HubMap({ hubs = [], routes = [], activePath = [], mstEdges = [] }) {
  const activePairs = new Set();
  for (let i = 0; i + 1 < activePath.length; i++) {
    activePairs.add(`${activePath[i]}->${activePath[i + 1]}`);
  }

  const mstPairs = new Set();
  for (const e of mstEdges) {
    mstPairs.add(`${e.from}-${e.to}`);
    mstPairs.add(`${e.to}-${e.from}`);
  }

  return (
    <svg viewBox="0 0 500 365" className="map-svg" role="img" aria-label="14-hub network map">
      {routes.map((r, idx) => {
        const a = HUB_COORDS[r.from];
        const b = HUB_COORDS[r.to];
        if (!a || !b) return null;

        const isActive =
          activePairs.has(`${r.from}->${r.to}`) ||
          (r.bidirectional && activePairs.has(`${r.to}->${r.from}`));
        const isMst = mstPairs.has(`${r.from}-${r.to}`);

        const stroke = isActive ? '#4d8dff' : isMst ? '#5ed39a' : '#2d3544';
        const strokeWidth = isActive ? 3.2 : isMst ? 2.4 : 1.2;

        return (
          <g key={`${r.from}-${r.to}-${idx}`}>
            <line
              x1={a.x}
              y1={a.y}
              x2={b.x}
              y2={b.y}
              stroke={stroke}
              strokeWidth={strokeWidth}
              strokeDasharray={r.bidirectional ? undefined : '4 3'}
            />
            <text
              x={(a.x + b.x) / 2}
              y={(a.y + b.y) / 2 - 3}
              fill={isActive ? '#e9ecf2' : '#738096'}
              fontSize="8.5"
              textAnchor="middle"
            >
              {r.cost}
            </text>
          </g>
        );
      })}

      {hubs.map((h) => {
        const pt = HUB_COORDS[h.name];
        if (!pt) return null;
        const inPath = activePath.includes(h.name);
        return (
          <g key={h.id}>
            <circle
              cx={pt.x}
              cy={pt.y}
              r={inPath ? 6.5 : 4.8}
              fill={inPath ? '#4d8dff' : '#181c24'}
              stroke={inPath ? '#e9ecf2' : '#9aa3b2'}
              strokeWidth="1.6"
            />
            <text
              x={pt.x}
              y={pt.y + 14}
              fill={inPath ? '#e9ecf2' : '#cbd3e1'}
              fontSize="9.5"
              fontWeight={inPath ? '700' : '500'}
              textAnchor="middle"
            >
              {h.name}
            </text>
          </g>
        );
      })}
    </svg>
  );
}
