package handlers

import (
	"encoding/json"
	"net/http"
	"vm-machine/models"
)

func GetVariableHandler(repo *models.Repository) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		modelName := r.URL.Query().Get("model")
		varName := r.URL.Query().Get("variable")

		variable, err := repo.GetVariable(modelName, varName)
		if err != nil {
			http.Error(w, err.Error(), http.StatusNotFound)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(variable)
	}
}
