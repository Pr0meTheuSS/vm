package main

import (
	"log"
	"net/http"
	"vm-machine/handlers"
	"vm-machine/models"
	"vm-machine/utils"
)

func main() {
	repo := models.NewRepository()

	http.HandleFunc("/api/generate-graph", utils.CORS(handlers.GenerateGraphHandler(repo)))
	http.HandleFunc("/api/get-variable", utils.CORS(handlers.GetVariableHandler(repo)))
	http.HandleFunc("/api/get-function", utils.CORS(handlers.GetFunctionHandler(repo)))
	http.HandleFunc("/api/calculate-order", handlers.CalculateOrderHandler(repo))
	http.HandleFunc("/api/generate-cpp", handlers.GenerateCppHandler(repo))

	log.Println("Server running on http://localhost:8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
