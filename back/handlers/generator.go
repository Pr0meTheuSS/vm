package handlers

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"vm-machine/models"
)

type GenerateCppRequest struct {
	ModelName      string `json:"modelName"`
	TargetVariable string `json:"targetVariable"`
}

type GenerateCppResponse struct {
	Code string `json:"code"`
}

// GenerateCppHandler обрабатывает запросы на генерацию C++ кода.
func GenerateCppHandler(repo *models.Repository) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		log.Println("Received request for /generateCpp")

		if r.Method != http.MethodPost {
			log.Printf("Invalid method: %s. Only POST is allowed.\n", r.Method)
			http.Error(w, "Only POST method is allowed", http.StatusMethodNotAllowed)
			return
		}

		log.Println("Decoding request body...")
		var request GenerateCppRequest
		if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
			log.Printf("Failed to decode request: %v\n", err)
			http.Error(w, "Invalid request body", http.StatusBadRequest)
			return
		}

		log.Printf("Request decoded: ModelName=%s, TargetVariable=%s\n", request.ModelName, request.TargetVariable)
		if request.ModelName == "" || request.TargetVariable == "" {
			log.Println("Validation failed: modelName or targetVariable is empty")
			http.Error(w, "Both modelName and targetVariable are required", http.StatusBadRequest)
			return
		}

		log.Printf("Fetching model '%s'...\n", request.ModelName)
		model, err := repo.GetModel(request.ModelName)
		if err != nil {
			log.Printf("Model not found: %v\n", err)
			http.Error(w, "Model not found", http.StatusNotFound)
			return
		}

		log.Printf("Calculating variable order for target '%s'...\n", request.TargetVariable)
		graph, err := repo.CalculateVariableOrder(request.ModelName, request.TargetVariable)
		if err != nil {
			log.Printf("Error calculating variable order: %v\n", err)
			http.Error(w, fmt.Sprintf("Error calculating order: %v", err), http.StatusInternalServerError)
			return
		}

		log.Printf("Fetching variables for the model '%s'...\n", request.ModelName)
		variables, err := repo.GetVariables(request.ModelName)
		if err != nil {
			log.Printf("Error fetching variables: %v\n", err)
			http.Error(w, "Error fetching variables", http.StatusInternalServerError)
			return
		}

		log.Printf("Generating C++ code...\n")
		code := models.GenerateCppFromGraph(*graph, variables, repo, model.Name)

		log.Println("Encoding and sending response...")
		response := GenerateCppResponse{Code: code}
		w.Header().Set("Content-Type", "application/json")
		if err := json.NewEncoder(w).Encode(response); err != nil {
			log.Printf("Error encoding response: %v\n", err)
			http.Error(w, "Error encoding response", http.StatusInternalServerError)
			return
		}

		log.Println("Response successfully sent.")
	}
}
