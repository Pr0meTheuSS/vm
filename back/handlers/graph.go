package handlers

import (
	"encoding/json"
	"net/http"
	"vm-machine/models"
)

func GenerateGraphHandler(repo *models.Repository) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		var model models.Model

		if err := json.NewDecoder(r.Body).Decode(&model); err != nil {
			http.Error(w, "Invalid input", http.StatusBadRequest)
			return
		}

		repo.SaveModel(&model)
		graph := parseModel(model)

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(graph)
	}
}

func parseModel(model models.Model) *models.Graph {
	return &models.Graph{Variables: model.Variables, Functions: model.Functions}
}
