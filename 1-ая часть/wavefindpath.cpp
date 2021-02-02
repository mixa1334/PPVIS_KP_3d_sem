#include <iostream>
#include <vector>
#include <string>
#include "cpp/sc_addr.hpp"
#include "cpp/sc_memory.hpp"
#include "cpp/sc_iterator.hpp"
#include "utils.h"


ScAddr graph, rrel_arcs, rrel_nodes,nrel_weight;


int get_weight(const std::unique_ptr<ScMemoryContext> &context, ScAddr arc)  {
    ScIterator5Ptr it = context->Iterator5(arc, ScType::EdgeDCommon, ScType(0), ScType::EdgeAccessConstPosPerm,
                                           nrel_weight);
    if (it->Next()) {
        return  stoi(context->HelperGetSystemIdtf(it->Get(2)));
    }
    return 0;
}

void get_edge_vertexes (const std::unique_ptr<ScMemoryContext>& context,ScAddr edge, ScAddr &v1, ScAddr &v2){
    v1 = context->GetEdgeSource(edge);
    v2 = context->GetEdgeTarget(edge);
}

void print_graph (const std::unique_ptr<ScMemoryContext>& context){

    ScAddr arcs, v1, v2 ;
    printEl(context, graph);
    std::cout << std::endl << "----------------------" << std::endl;

    ScIterator5Ptr it = context->Iterator5(graph,ScType::EdgeAccessConstPosPerm,ScType(0),ScType::EdgeAccessConstPosPerm,rrel_arcs);

    if (it->Next()) {
        arcs = it->Get(2);

        ScIterator3Ptr arcs_it = context->Iterator3(arcs,ScType::EdgeAccessConstPosPerm,ScType(0));

        while (arcs_it->Next()) {

            ScAddr t_arc = arcs_it->Get(2);

            get_edge_vertexes (context,t_arc, v1, v2);

            printEl(context, v1);
            std::cout << " <= ";
            std::cout<<get_weight(context,t_arc);
            std::cout<<" =>";
            printEl(context, v2);
            std::cout << std::endl;
        }
    }
}

bool find_vertex_in_set (const std::unique_ptr<ScMemoryContext>& context,ScAddr element, ScAddr set){
    bool find = false;
    ScIterator3Ptr location = context->Iterator3(set,ScType::EdgeDCommon,ScType(0));

    while (location->Next()) {
        ScAddr loc = location->Get(2);

        if (loc != element) {
            find = false;
            continue;
        } else {
            find = true;
            break;
        }
    }
    return find;
}

ScAddr get_other_vertex_incidence_edge (const std::unique_ptr<ScMemoryContext>& context,ScAddr edge, ScAddr vertex){
    ScAddr v1, v2, empty;
    get_edge_vertexes(context,edge, v1, v2);
    if (vertex == v1 || vertex == v2) {
        if (vertex == v1) {
            return v2;
        } else {
            return v1;
        }
    }
    return empty;
}

int find_path(const std::unique_ptr<ScMemoryContext>& context,ScAddr& start,ScAddr& end,ScAddr& printed_vertex)
{
    int path=1000000;

    ScAddr t=context->CreateEdge(ScType::EdgeDCommon,printed_vertex,start);

    ScIterator5Ptr arcs = context->Iterator5(graph,ScType::EdgeAccessConstPosPerm,ScType(0),ScType::EdgeAccessConstPosPerm,rrel_arcs);

    if (arcs->Next()) {
        ScAddr set_arcs = arcs->Get(2);
        ScIterator3Ptr it_arc = context->Iterator3(set_arcs,ScType::EdgeAccessConstPosPerm,ScType(0));

        while (it_arc->Next()) {
            int temp=0;
            ScAddr t_arc = it_arc->Get(2);
            ScAddr other_vertex = get_other_vertex_incidence_edge(context,t_arc, start);
            if (!other_vertex.IsValid()) {
                continue;
            }
            if(!find_vertex_in_set(context,other_vertex,printed_vertex)) {
                if (other_vertex == end) {
                    temp += get_weight(context,t_arc);
                } else {
                    temp += get_weight(context,t_arc);
                    temp += find_path(context, other_vertex, end, printed_vertex);
                }
                if (temp < path) {
                    path = temp;
                }
            }
        }
    }
    context->EraseElement(t);

    return path;
}

int find_eccentricity_of_vertex(const std::unique_ptr<ScMemoryContext>& context,ScAddr& vertex)
{
    int path=0;

    ScIterator5Ptr it = context->Iterator5(graph,ScType::EdgeAccessConstPosPerm,ScType(0),ScType::EdgeAccessConstPosPerm,rrel_nodes);

    if (it->Next()) {
        ScAddr nodes = it->Get(2);

        ScIterator3Ptr nodes_it = context->Iterator3(nodes,ScType::EdgeAccessConstPosPerm,ScType(0));

        while (nodes_it->Next()) {
            ScAddr printed_vertex = context->CreateNode(ScType::Const);
            int temp=0;
            ScAddr t_node = nodes_it->Get(2);
            if (t_node == vertex) {
                continue;
            }
            temp = find_path(context, vertex, t_node, printed_vertex);
            if (temp > path) {
                path = temp;
            }
            context->EraseElement(printed_vertex);
        }
    }
    return path;
}

void find_peripheral_vert(const std::unique_ptr<ScMemoryContext>& context)
{
    int d=0;

    std::vector<std::pair<ScAddr,int>> periph_vert;

    ScIterator5Ptr it= context->Iterator5(graph,ScType::EdgeAccessConstPosPerm,ScType(0),ScType::EdgeAccessConstPosPerm,rrel_nodes);

    if (it->Next()) {
        ScAddr nodes = it->Get(2);

        ScIterator3Ptr nodes_it = context->Iterator3(nodes,ScType::EdgeAccessConstPosPerm,ScType(0));

        while (nodes_it->Next()) {
            int exc;
            ScAddr cur = nodes_it->Get(2);
            exc=find_eccentricity_of_vertex(context,cur);
            if(exc>=d) {
                d = exc;
                periph_vert.push_back(std::pair<ScAddr,int>(cur,exc));
            }
        }
    }

    for(auto& p:periph_vert)
    {
        if(p.second==d)
        {
            printEl(context,p.first);
            std::cout<<" - "<<p.second<<"    ";
        }
    }
}

void run_test(const std::unique_ptr<ScMemoryContext>& context,std::string number_test)
{
    std::string gr = "Gx";
    gr[1] = number_test[0];
    graph = context->HelperResolveSystemIdtf(gr);
    rrel_arcs = context->HelperResolveSystemIdtf("rrel_arcs");
    rrel_nodes = context->HelperResolveSystemIdtf("rrel_nodes");
    nrel_weight = context->HelperResolveSystemIdtf("nrel_weight");

    std::cout<<std::endl<<std::endl;
    std::cout << "Graph: ";
    print_graph(context);

    std::cout<<std::endl<<"Peripheral vertexs: "<<std::endl;
    find_peripheral_vert(context);

    std::cout<<std::endl;
}

int main()
{
    ScMemory::LogMute();
    sc_memory_params params;

    sc_memory_params_clear(&params);
    params.repo_path = "/home/mixa/ostis/ostis-0.5.0/ostis-example-app/ostis/kb.bin";
    params.config_file = "/home/mixa/ostis/ostis-0.5.0/ostis-example-app/ostis/config/sc-web.ini";
    params.ext_path = "/home/mixa/ostis/ostis-0.5.0/ostis-example-app/ostis/sc-machine/bin/extensions";
    params.clear = SC_FALSE;

    ScMemory mem;
    mem.Initialize(params);

    const std::unique_ptr<ScMemoryContext> context(new ScMemoryContext(sc_access_lvl_make_max,"example"));

    run_test(context,"1");
    run_test(context,"2");
    run_test(context,"3");
    run_test(context,"4");
    run_test(context,"5");

    std::cout<<std::endl << "The end" << std::endl;

    mem.Shutdown(true);

    return 0;
}