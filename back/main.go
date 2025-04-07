package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"strings"
)

const (
	Scalar = 0
)

type Variable struct {
	Name  string  `json:"name"`
	Type  string  `json:"type"`
	Value *string `json:"value,omitempty"`
	Dim   int     `json:"dim"`
	Size  []int   `json:"size"`
}

type Operation struct {
	OutputName    string   `json:"output_name"`
	OutputType    string   `json:"output_type"`
	Inputs        []string `json:"inputs"`
	Code          string   `json:"code"`
	OperationName string   `json:"operation_name"`
}

type Model struct {
	Variables  []Variable  `json:"variables"`
	Operations []Operation `json:"operations"`
}

type Node struct {
	Name       string
	DependsOn  []string
	IsComputed bool
	Code       string
}

type MemoryBlock struct {
	Address string
	Size    int
}

func BuildGraph(model Model) map[string]*Node {
	graph := make(map[string]*Node)

	// Строим граф зависимостей для переменных
	for _, v := range model.Variables {
		isComputed := v.Value == nil
		graph[v.Name] = &Node{
			Name:       v.Name,
			DependsOn:  []string{},
			IsComputed: isComputed,
		}
	}

	// Строим граф зависимостей для операций
	for _, op := range model.Operations {
		if _, exists := graph[op.OutputName]; !exists {
			graph[op.OutputName] = &Node{Name: op.OutputName, DependsOn: []string{}, IsComputed: true}
		}
		graph[op.OutputName].DependsOn = op.Inputs
		graph[op.OutputName].Code = op.Code
	}

	return graph
}

func GenerateCCode(model Model, graph map[string]*Node) string {
	var sb strings.Builder

	sb.WriteString("#include <stdio.h>\n#include <stdlib.h>\n\n")

	// Генерация памяти для переменных
	for _, v := range model.Variables {
		if v.Dim == Scalar {
			// Скаляр
			sb.WriteString(fmt.Sprintf("%s %s;\n", v.Type, v.Name))
		}
	}

	// Генерация функций на основе операций
	for _, op := range model.Operations {
		sb.WriteString(fmt.Sprintf("\n// Функция для вычисления %s\n", op.OperationName))
		sb.WriteString(fmt.Sprintf("%s %s(", op.OutputType, op.OperationName))
		for i, input := range op.Inputs {
			sb.WriteString(fmt.Sprintf("%s %s", model.Variables[i].Type, input))
			if i != len(op.Inputs)-1 {
				sb.WriteString(", ")
			}
		}
		sb.WriteString(") {\n")
		sb.WriteString(op.Code + "\n")
		sb.WriteString("}\n")
	}

	// Основная функция
	sb.WriteString("int main() {\n")

	// Загружаем известные значения
	for _, v := range model.Variables {
		if v.Value != nil {
			if strings.HasPrefix(*v.Value, "local:") {
				val := strings.TrimPrefix(*v.Value, "local:")
				sb.WriteString(fmt.Sprintf("    %s = %s;\n", v.Name, val))
			} else {
				sb.WriteString(fmt.Sprintf("    printf(\"Fetching %s from %s...\\n\");\n", v.Name, *v.Value))
			}
		}
	}

	// Выполняем вычисления по операциям
	for _, op := range model.Operations {
		isComputed := true
		for i, variable := range model.Variables {
			if variable.Name == op.OutputName && model.Variables[i].Value == nil {
				isComputed = false
			}
		}
		if isComputed {
			continue
		}
		sb.WriteString(fmt.Sprintf("\t%s = %s(", op.OutputName, op.OperationName))
		for i, arg := range op.Inputs {
			sb.WriteString(fmt.Sprintf("%s", arg))
			if i != len(op.Inputs)-1 {
				sb.WriteString(fmt.Sprint(", "))
			}
		}
		sb.WriteString(");\n")

	}

	sb.WriteString("\n    return 0;\n}\n")
	return sb.String()
}

func main() {
	// Проверяем аргументы командной строки для указания пути к файлу
	if len(os.Args) < 2 {
		log.Fatal("Please provide the path to the JSON file as an argument.")
	}

	// Загружаем JSON из файла
	filePath := os.Args[1]
	file, err := os.Open(filePath)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	var model Model
	decoder := json.NewDecoder(file)
	err = decoder.Decode(&model)
	if err != nil {
		log.Fatal(err)
	}

	// Строим граф зависимостей
	graph := BuildGraph(model)

	// Генерация C-кода
	cCode := GenerateCCode(model, graph)

	// Запись сгенерированного кода в файл
	outputFile := "generated_program.c"
	file, err = os.Create(outputFile)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	_, err = file.WriteString(cCode)
	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("C-код сгенерирован в файл %s\n", outputFile)
}
