#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graph {

template <typename Weight>
class Router {
private:
    using Graph = DirectedWeightedGraph<Weight>;

public:
    // Принимает ссылку на существующий граф с маршрутами
    // Проходит в два этапа:
    // - заполение матрицы вершин и ребер из графа,
    // - оптимизация расстояний между вершинами в матрице
    explicit Router(const Graph& graph);

    // Информация о маршруте: общий размер и список id соответствующих ребер графа
    struct RouteInfo {
        Weight weight;
        std::vector<EdgeId> edges;
    };

    // Возвращает общую длительности и список ребер для оптимального маршрута
    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;

private:
    struct RouteInternalData {
        Weight weight;
        std::optional<EdgeId> prev_edge;
    };

    // Список маршрутов. Каждый маршрут содержит список ребер в маршруте
    using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

    // Заполнение матрицы вершин routes_internal_data_, где первый индекс - вершины отправления,
    // второй - вершины прибытия
    void InitializeRoutesInternalData(const Graph& graph) {
        const size_t vertex_count = graph.GetVertexCount();

        // Проход по id вершин. Заполенение матрицы вершин
        for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
            // для одной и той же вершины задается нулевое ребро
            routes_internal_data_[vertex][vertex] = RouteInternalData{ZERO_WEIGHT, std::nullopt};

            // Проход по id ребер
            for (const EdgeId edge_id : graph.GetIncidentEdges(vertex)) {
                // Возврат информации о ребре и проверка ее валидности
                const auto& edge = graph.GetEdge(edge_id);
                if (edge.weight < ZERO_WEIGHT) {
                    throw std::domain_error("Edges' weights should be non-negative");
                }

                // Переход в ячейку, соответствующую ребру между вершиной vertex
                // и конечной для этого ребра
                auto& route_internal_data = routes_internal_data_[vertex][edge.to];

                // Задание расстояния отличного от нуля, или сокращение его
                if (!route_internal_data || route_internal_data->weight > edge.weight) {
                    route_internal_data = RouteInternalData{edge.weight, edge_id};
                }
            }
        }
    }

    // Проверка оптимальности маршрута между двух вершин через третью (среднюю) вершину
    void RelaxRoute(VertexId vertex_from, VertexId vertex_to, const RouteInternalData& route_from,
                    const RouteInternalData& route_to) {
        // Получение ссылки на возможный граф для двух вершин, соединенных с
        // со средней вершиной
        auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];

        // Получение расстояние при движении черех среднюю вершину
        const Weight candidate_weight = route_from.weight + route_to.weight;

        // Определение оптимального расстояния между двух вершин: напрямую или
        // через среднюю вершину
        if (!route_relaxing || candidate_weight < route_relaxing->weight) {
            route_relaxing = {candidate_weight,
                              route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge};
        }
    }

    // Оптимизация матрицы маршрутов путем проверки длительности прямого пути между двух вершин
    // и с проходом через третью (среднюю) вершину
    void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
        for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
            // Если в ячейке матрицы есть граф для перемещения до vertex_trough
            if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) {
                for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                    // И если в ячейке матрицы есть граф для перемещения от vertex_trough
                    if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) {
                        RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                    }
                }
            }
        }
    }

    // Постоянная для обозначения пустого веса ребра
    static constexpr Weight ZERO_WEIGHT{};

    // Ссылка на граф со всеми маршрутами
    const Graph& graph_;

    // Матрица расстояний. Две оси - вершины графа (отправление и прибытие соответственно).
    // Ячейки - ребра графа
    RoutesInternalData routes_internal_data_;
};

template <typename Weight>
Router<Weight>::Router(const Graph& graph)
    : graph_(graph)
    , routes_internal_data_(graph.GetVertexCount(),
                            std::vector<std::optional<RouteInternalData>>(graph.GetVertexCount()))
{
    InitializeRoutesInternalData(graph);

    const size_t vertex_count = graph.GetVertexCount();
    for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
        RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
    }
}

template <typename Weight>
std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from,
                                                                             VertexId to) const {
    // Получение оптимизированного ребра для остановок из матрицы
    const auto& route_internal_data = routes_internal_data_.at(from).at(to);

    // Возврат нулевого указателя в случае невозможности построить маршрут
    if (!route_internal_data) {
        return std::nullopt;
    }

    const Weight weight = route_internal_data->weight;

    // Заполнение списка ребер маршрута по узлам матрицы (в обратном порядке)
    std::vector<EdgeId> edges;
    for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
         edge_id;
         edge_id = routes_internal_data_[from][graph_.GetEdge(*edge_id).from]->prev_edge)
    {
        edges.push_back(*edge_id);
    }

    std::reverse(edges.begin(), edges.end());

    return RouteInfo{weight, std::move(edges)};
}

}  // namespace graph