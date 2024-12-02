import React, { useState } from 'react';
import { Button, TextField, Typography, Box, Alert } from '@mui/material';
import Graph from './Graph';

const App = () => {
  const [model, setModel] = useState('');
  const [graphData, setGraphData] = useState(null);
  const [error, setError] = useState(null);

  const handleModelChange = (e) => {
    setModel(e.target.value);
  };

  const handleGenerateGraph = async () => {
    try {
      setError(null);
      setGraphData(null);

      // Парсинг модели
      const parsedModel = JSON.parse(model);
      console.log('Отправляемое тело запроса:', JSON.stringify(parsedModel, null, 2));

      // Отправка модели на сервер
      const response = await fetch('http://localhost:8080/api/generate-graph', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(parsedModel),
      });

      if (!response.ok) {
        throw new Error(`Ошибка сети: ${response.status}`);
      }

      const data = await response.json();

      // Разделение переменных
      const knownVars = parsedModel.vars.filter((v) => v.value !== undefined); // Известные переменные
      const unknownVars = parsedModel.vars.filter((v) => v.value === undefined); // Неизвестные переменные

      // Построение данных для двудольного графа
      const graphData = {
        nodes: [
          ...knownVars.map((v) => ({ id: v.name, group: 'known' })), // Левый слой
          ...unknownVars.map((v) => ({ id: v.name, group: 'unknown' })), // Правый слой
          ...parsedModel.functions.map((f) => ({ id: f.name, group: 'function' })), // Функции
        ],
        links: [
          ...parsedModel.functions.flatMap((f) =>
            f.inputs.map((input) => ({
              source: input,
              target: f.name,
            }))
          ),
          ...parsedModel.functions.map((f) => ({
            source: f.name,
            target: f.output,
          })),
        ],
      };

      console.log('Двудольный граф:', graphData);
      setGraphData(graphData);
    } catch (error) {
      if (error instanceof SyntaxError) {
        setError('Некорректный JSON. Пожалуйста, проверьте формат ввода.');
      } else {
        setError(`Ошибка при генерации графа: ${error.message}`);
      }
      console.error('Ошибка:', error);
    }
  };

  return (
    <Box sx={{ padding: 3 }}>
      <Typography variant="h4" gutterBottom>
        Генератор двудольного графа
      </Typography>

      <TextField
        label="Введите модель"
        multiline
        rows={6}
        value={model}
        onChange={handleModelChange}
        fullWidth
        variant="outlined"
        sx={{ marginBottom: 2 }}
      />

      <Button
        variant="contained"
        color="primary"
        onClick={handleGenerateGraph}
        sx={{ marginBottom: 2 }}
      >
        Сгенерировать граф
      </Button>

      {error && (
        <Alert severity="error" sx={{ marginBottom: 2 }}>
          {error}
        </Alert>
      )}

      {graphData ? (
        <Graph data={graphData} />
      ) : (
        <Typography variant="body1" sx={{ marginTop: 2 }}>
          Граф пока не создан. Введите модель и нажмите "Сгенерировать граф".
        </Typography>
      )}
    </Box>
  );
};

export default App;
