package models

import (
	"bytes"
	"fmt"
)

func GenerateCppFromGraph(graph CalculationGraph, variables []VariableNode, modelRepo *Repository, modelName string) string {
	var buffer bytes.Buffer

	// Добавление заголовка
	addHeader(&buffer)

	// Объявление типов
	addTypedefs(&buffer, variables)

	// Объявление и реализация функций
	addFunctionsFromGraph(&buffer, graph, modelRepo, variables, modelName)

	// Генерация функции main
	addMain(&buffer, graph, variables, modelRepo)

	return buffer.String()
}

// addHeader добавляет заголовок файла.
func addHeader(buffer *bytes.Buffer) {
	buffer.WriteString("#include <iostream>")
	buffer.WriteString("#include <cstdlib>\n\n")
}

// addTypedefs добавляет typedef'ы для переменных.
func addTypedefs(buffer *bytes.Buffer, variables []VariableNode) {
	for _, variable := range variables {
		buffer.WriteString(fmt.Sprintf("typedef %s %s;\n", variable.Type, capitalize(variable.Name)))
	}
	buffer.WriteString("\n")
}

// addFunctionsFromGraph добавляет функции, определённые в графе.
func addFunctionsFromGraph(buffer *bytes.Buffer, graph CalculationGraph, modelRepo *Repository, variables []VariableNode, modelName string) {
	for _, node := range graph.Nodes {
		// Получаем функцию из репозитория по имени модели и операции
		function, err := modelRepo.GetFunction(modelName, node.Operation)
		if err != nil {
			// Если функция не найдена, пропускаем её
			continue
		}

		// Добавление объявления функции
		buffer.WriteString(fmt.Sprintf("%s %s(", capitalize(getVarType(node.Outputs[0], variables)), node.Operation))
		for i, input := range node.Inputs {
			if i > 0 {
				buffer.WriteString(", ")
			}
			buffer.WriteString(fmt.Sprintf("%s %s", capitalize(getVarType(input, variables)), input))
		}
		buffer.WriteString(") {\n")

		// Добавляем тело функции из репозитория
		buffer.WriteString(function.Body)

		buffer.WriteString("\n}\n\n")
	}
}

// addMain добавляет функцию main.
func addMain(buffer *bytes.Buffer, graph CalculationGraph, variables []VariableNode, modelRepo *Repository) {
	buffer.WriteString("int main(int argc, char* argv[]) {\n")

	// Инициализация переменных
	for _, variable := range variables {
		if variable.Value != "" {
			buffer.WriteString(fmt.Sprintf("    %s %s = %s;\n", capitalize(variable.Type), variable.Name, variable.Value))
		}
	}

	// Добавление шагов вычисления из графа
	for _, node := range graph.Nodes {
		outputs := node.Outputs[0] // Для простоты предположим одну выходную переменную
		buffer.WriteString(fmt.Sprintf("    %s = %s(", outputs, node.Operation))
		for i, input := range node.Inputs {
			if i > 0 {
				buffer.WriteString(", ")
			}
			buffer.WriteString(input)
		}
		buffer.WriteString(");\n")
	}

	buffer.WriteString("\n    return EXIT_SUCCESS;\n")
	buffer.WriteString("}\n")
}

func getVarType(name string, vars []VariableNode) string {
	for _, v := range vars {
		if v.Name == name {
			return v.Type
		}
	}
	return "unknown"
}

func capitalize(str string) string {
	if len(str) == 0 {
		return str
	}
	return string(str[0]-32) + str[1:]
}
