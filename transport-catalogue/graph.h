#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

// id вершины
using VertexId = size_t;

// id ребра
using EdgeId = size_t;


template <typename Weight>
struct Edge {
    // Начальная вершина
    VertexId from;
    // Конечная вершина
    VertexId to;
    // Вес ребра (для маршрута - время)
    Weight weight;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    // Список id ребер
    using IncidenceList = std::vector<EdgeId>;

    // Пара итераторов на массив ребер, выходящих из узла
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;

    // Конструктор по предполагаемому количеству пересекаемых остановок
    explicit DirectedWeightedGraph(size_t vertex_count);

    // Добавление ребра в граф. Возвращает присвоенное значение id ребра в графе
    EdgeId AddEdge(const Edge<Weight>& edge);

    // Возврат количества вершин графа
    size_t GetVertexCount() const;

    // Возврат количества ребер графа
    size_t GetEdgeCount() const;

    // Возвращает ссылку на ребро по его id
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;

    // Возвращает пару итераторов на массив ребер, исходящих из id узла vertex
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

private:
    // Массив всех ребер
    std::vector<Edge<Weight>> edges_;

    // Массив всех вершин, со списком исходящих ребер для каждой вершины
    std::vector<IncidenceList> incidence_lists_;
};


template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    // Добаление ребра в общий список ребер
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    // Добавление ребра в список исходящих ребер для узла
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph