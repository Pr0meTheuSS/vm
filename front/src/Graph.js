import React, { useEffect, useRef } from 'react';
import { Network } from 'vis-network';

const Graph = ({ data }) => {
  const networkRef = useRef(null);
  const networkInstance = useRef(null);

  useEffect(() => {
    if (networkRef.current && data) {
      const nodes = [];
      const edges = [];

      // Обработка известных переменных
      if (data.nodes) {
        data.nodes.forEach((node) => {
          nodes.push({
            id: node.id,
            label: node.id,
            group: node.group,
          });
        });
      }

      // Обработка связей
      if (data.links) {
        data.links.forEach((link) => {
          edges.push({
            from: link.source,
            to: link.target,
            arrows: 'to',
          });
        });
      }

      const graphData = { nodes, edges };
      const options = {
        groups: {
          known: { color: { background: 'lightblue' }, shape: 'ellipse', font: { color: 'black' } },
          unknown: { color: { background: 'pink' }, shape: 'ellipse', font: { color: 'black' } },
          function: { color: { background: 'lightgreen' }, shape: 'box', font: { color: 'black' } },
        },
        edges: {
          color: { color: 'black' },
          arrows: {
            to: { enabled: true, scaleFactor: 1.0 },
          },
        },
        layout: {
          hierarchical: {
            enabled: true,
            direction: 'LR', // Слева направо
            sortMethod: 'directed',
          },
        },
        physics: false, // Отключение физического взаимодействия для упрощения отображения
      };

      if (!networkInstance.current) {
        networkInstance.current = new Network(networkRef.current, graphData, options);
      } else {
        networkInstance.current.setData(graphData);
        networkInstance.current.setOptions(options);
      }
    }

    return () => {
      if (networkInstance.current) {
        networkInstance.current.destroy();
        networkInstance.current = null;
      }
    };
  }, [data]);

  return (
    <div ref={networkRef} style={{ height: '500px', border: '1px solid black' }}>
      {data ? null : <p>Нет данных для отображения графа.</p>}
    </div>
  );
};

export default Graph;
