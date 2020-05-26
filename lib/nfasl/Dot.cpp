#include "nfasl/Dot.hpp"
#include "nfasl/Nfasl.hpp"

#include "Algo.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/graph/graphviz.hpp>
#include <fstream>
#include <set>

namespace nfasl {


  //using namespace boost;

  /*
struct VertexP { std::string tag; };
struct EdgeP { std::string symbol; };
struct GraphP { std::string orientation; };

typedef adjacency_list<vecS, vecS, directedS, VertexP, EdgeP, GraphP> Graph;

int main() {
    Graph g(GraphP{"LR"});
    // Then fill the graph
    add_edge(
        add_vertex(VertexP{ "tag1" }, g),
        add_vertex(VertexP{ "tag2" }, g),
        EdgeP{ "symbol" }, g
    );

    {
        std::ofstream dot_file("automaton.dot");
        dynamic_properties dp;
        dp.property("node_id", get(&VertexP::tag, g));
        dp.property("label", get(&VertexP::tag, g));
        dp.property("label", get(&EdgeP::symbol, g));
        dp.property("rankdir", boost::make_constant_property<Graph*>(std::string("LR")));
        dp.property("dummy", boost::make_function_property_map<Graph*>([](Graph const* g) { return g->m_property->orientation; }));

        write_graphviz_dp(dot_file, g, dp);
    }
}
  */
using namespace boost;
  //typedef boost::GraphvizDigraph Graph;
typedef adjacency_list<vecS, vecS, bidirectionalS,
                       property<vertex_color_t, default_color_type>//,
                       //                       property<vertex_shape_t, default_shape_type>
                       > Graph;
typedef graph_traits<Graph>::vertex_descriptor VertexDescriptor;

typedef size_t Name;

enum NodeType
  {
   NodeType_Initial = 0x1,
   NodeType_Final = 0x2
  };

class DotBuilder {
  Graph& graph;
  std::map<Name, VertexDescriptor> vertices;
public:
  DotBuilder(Graph& graph_) : graph(graph_) {}

  void addVertex(Name name, size_t typ) {
    assert(vertices.find(name) == vertices.end());
    // Add the vertex
    VertexDescriptor vertex = add_vertex(graph);
    // Add its property
    #if 0
    const boost::property_map<Graph, boost::vertex_attribute_t>::
        type& vertAttrMap = boost::get(boost::vertex_attribute, graph);
    vertAttrMap[vertex]["label"] = boost::lexical_cast<std::string>(name);
    if (typ | NodeType_Initial) {
      vertAttrMap[vertex]["shape"] = "doublecircle";
    } else {
      vertAttrMap[vertex]["shape"] = "circle";
    }
    if (typ | NodeType_Final) {
      vertAttrMap[vertex]["color"] = "green";
    }
    #endif
    vertices[name] = vertex;
  }

  void addEdge(Name src, Name dst, Predicate phi) {
    add_edge(vertices.at(src), vertices.at(dst), graph);
  }
};

static void toDot(const Nfasl& a, Graph& graph) {
  DotBuilder builder{graph};
  for (size_t q = 0; q < a.stateCount; ++q) {
    size_t typ = 0;
    if (q == a.initial) {
      typ |= NodeType_Initial;
    }
    if (set_member(a.finals, q)) {
      typ |= NodeType_Final;
    }
    builder.addVertex(q, typ);
  }
  for (size_t q = 0; q < a.stateCount; ++q) {
    for (auto const& rule : a.transitions[q]) {
      builder.addEdge(q, rule.state, rule.phi);
    }
  }
}

void toDot(const Nfasl& a, const std::string& file) {
  Graph graph;
  toDot(a, graph);

  std::ofstream stream{file};
  boost::write_graphviz(stream, graph);
}

} // namespace nfasl
