package handlers

import (
	"encoding/json"
	"net/http"
	"vm-machine/models"
)

func GetFunctionHandler(repo *models.Repository) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		modelName := r.URL.Query().Get("model")
		funcName := r.URL.Query().Get("function")

		function, err := repo.GetFunction(modelName, funcName)
		if err != nil {
			http.Error(w, err.Error(), http.StatusNotFound)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(function)
	}
}
