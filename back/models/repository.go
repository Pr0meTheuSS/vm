package models

import (
	"errors"
	"sync"
)

type Repository struct {
	mu     sync.Mutex
	models map[string]*Model
}

func NewRepository() *Repository {
	return &Repository{models: make(map[string]*Model)}
}

func (repo *Repository) SaveModel(model *Model) {
	repo.mu.Lock()
	defer repo.mu.Unlock()
	repo.models[model.Name] = model
}

func (repo *Repository) GetModel(name string) (*Model, error) {
	repo.mu.Lock()
	defer repo.mu.Unlock()
	if model, exists := repo.models[name]; exists {
		return model, nil
	}
	return nil, errors.New("model not found")
}

func (repo *Repository) GetVariable(modelName, variableName string) (*VariableNode, error) {
	model, err := repo.GetModel(modelName)
	if err != nil {
		return nil, err
	}

	for _, variable := range model.Variables {
		if variable.Name == variableName {
			return &variable, nil
		}
	}

	return nil, errors.New("variable not found")
}

func (repo *Repository) GetVariables(modelName string) ([]VariableNode, error) {
	model, err := repo.GetModel(modelName)
	if err != nil {
		return nil, err
	}
	return model.Variables, nil
}

func (repo *Repository) GetFunction(modelName, functionName string) (*FunctionNode, error) {
	model, err := repo.GetModel(modelName)
	if err != nil {
		return nil, err
	}

	for _, function := range model.Functions {
		if function.Name == functionName {
			return &function, nil
		}
	}

	return nil, errors.New("function not found")
}

func (repo *Repository) GetFunctionsByParams(modelName string, inputs []string, output string) ([]FunctionNode, error) {
	model, err := repo.GetModel(modelName)
	if err != nil {
		return nil, err
	}

	var matchedFunctions []FunctionNode
	for _, function := range model.Functions {
		if (output == "" || function.Output == output) && containsAll(function.Inputs, inputs) {
			matchedFunctions = append(matchedFunctions, function)
		}
	}

	return matchedFunctions, nil
}

func (repo *Repository) CalculateVariableOrder(modelName, targetVariable string) (*CalculationGraph, error) {
	model, err := repo.GetModel(modelName)
	if err != nil {
		return nil, err
	}

	var graph CalculationGraph
	visited := make(map[string]bool)

	// Рекурсивная функция для построения графа
	var resolve func(variableName string) error
	resolve = func(variableName string) error {
		if visited[variableName] {
			return nil
		}
		visited[variableName] = true

		// Проверяем, есть ли уже известное значение для переменной
		for _, variable := range model.Variables {
			if variable.Name == variableName && variable.Value != "" {
				graph.Nodes = append(graph.Nodes, CalculationNode{
					Operation: "Get",
					Inputs:    []string{variableName},
					Outputs:   []string{variableName},
				})
				return nil
			}
		}

		// Ищем функцию, которая производит данную переменную
		var producingFunc *FunctionNode
		for _, function := range model.Functions {
			if function.Output == variableName {
				producingFunc = &function
				break
			}
		}

		if producingFunc == nil {
			return errors.New("no function to calculate variable: " + variableName)
		}

		// Рекурсивно обрабатываем входные переменные функции
		for _, input := range producingFunc.Inputs {
			if err := resolve(input); err != nil {
				return err
			}
		}

		// Добавляем узел для текущей функции в граф
		graph.Nodes = append(graph.Nodes, CalculationNode{
			Operation: producingFunc.Name,
			Inputs:    producingFunc.Inputs,
			Outputs:   []string{variableName},
		})
		return nil
	}

	// Строим граф, начиная с целевой переменной
	if err := resolve(targetVariable); err != nil {
		return nil, err
	}

	return &graph, nil
}

func containsAll(slice, subset []string) bool {
	matches := make(map[string]bool)
	for _, s := range slice {
		matches[s] = true
	}
	for _, sub := range subset {
		if !matches[sub] {
			return false
		}
	}
	return true
}
