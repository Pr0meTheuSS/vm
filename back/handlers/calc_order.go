package handlers

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"vm-machine/models"
)

type CalculateOrderRequest struct {
	ModelName      string `json:"modelName"`
	TargetVariable string `json:"targetVariable"`
}

type CalculateOrderResponse struct {
	Order *models.CalculationGraph `json:"order"`
}

// CalculateOrderHandler обрабатывает запросы на расчет порядка переменных.
func CalculateOrderHandler(repo *models.Repository) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		log.Println("Received request for /calculateOrder")

		if r.Method != http.MethodPost {
			log.Printf("Invalid method: %s. Only POST is allowed.\n", r.Method)
			http.Error(w, "Only POST method is allowed", http.StatusMethodNotAllowed)
			return
		}

		log.Println("Decoding request body...")
		var request CalculateOrderRequest
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

		log.Printf("Calculating variable order for model '%s' and target '%s'...\n", request.ModelName, request.TargetVariable)
		order, err := repo.CalculateVariableOrder(request.ModelName, request.TargetVariable)
		if err != nil {
			log.Printf("Error calculating variable order: %v\n", err)
			http.Error(w, fmt.Sprintf("Error calculating order: %v", err), http.StatusInternalServerError)
			return
		}

		log.Printf("Calculation successful: %v\n", order)
		response := CalculateOrderResponse{Order: order}

		log.Println("Encoding and sending response...")
		w.Header().Set("Content-Type", "application/json")
		if err := json.NewEncoder(w).Encode(response); err != nil {
			log.Printf("Error encoding response: %v\n", err)
			http.Error(w, "Error encoding response", http.StatusInternalServerError)
			return
		}

		log.Println("Response successfully sent.")
	}
}
