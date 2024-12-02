package models

type Graph struct {
	Variables []VariableNode `json:"variables"`
	Functions []FunctionNode `json:"functions"`
}
