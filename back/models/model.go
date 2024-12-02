package models

type VariableNode struct {
	Name  string `json:"name"`
	Type  string `json:"type"`
	Value string `json:"value"`
}

type FunctionNode struct {
	Name   string   `json:"name"`
	Inputs []string `json:"inputs"`
	Output string   `json:"output"`
	Body   string   `json:"body"`
}

type Model struct {
	Name      string         `json:"name"`
	Variables []VariableNode `json:"vars"`
	Functions []FunctionNode `json:"functions"`
}

type CalculationNode struct {
	Operation string   // Имя операции (например, "calcSquare", "calcPerimeter")
	Inputs    []string // Входные переменные
	Outputs   []string // Выходные переменные
}

type CalculationGraph struct {
	Nodes []CalculationNode // Список узлов, упорядоченный по зависимости
}
