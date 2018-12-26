//===----------------------------------------------------------------------===//
//  Author : Premsai Chinthamreddy                                           //
//===--------------------------------------------------------------------===//
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
#include <vector>
#include <stack>
#include <set>
#include <utility>
#include <algorithm>

using namespace llvm;
namespace loopAnalysis{
    class node {
       public:
        int id;
        BasicBlock* bb;
        std::vector<node> edges;
        std::vector<node> back_edges;
       
        node(int id, BasicBlock* blk){
            this->id = id;
            bb = blk;
        }

        int number_of_out_edges() {
            return edges.size();
        }
        inline bool operator==(const node& n2){
            return this->bb == n2.bb;
        }
        inline bool operator<(const node& n2) {
            return this->bb < n2.bb;
        }

        std::vector<node> getPredecessors(){
            return back_edges;
        }

        void addPredecessors(node& n1) {
            back_edges.push_back(n1);
        }

    };
    struct cmpNode {
        bool operator()(const node& a, const node& b) const {
            return a.bb < b.bb;
        }
    };

    class flowgraph {
        public:
            std::vector<node> fg_nodes;
            std::map<node, std::vector<node>, cmpNode> edges;
            std::map<node, std::vector<node>, cmpNode> back_edges;
            static int nodeid;
            std::vector<std::pair<node,node>> loop;
            std::vector<std::vector<node>> loop_basic_block;
            std::map<int, std::vector<int>> loop_num_nodes;
            std::map<int, std::vector<node>> loop_nodes;
            std::vector<int> loop_start_nodes;

            node insert_node(BasicBlock *b) {
                node new_node(nodeid, b);
                if(std::find(fg_nodes.begin(), fg_nodes.end(), new_node) == fg_nodes.end()){
                    fg_nodes.push_back(new_node);
                    nodeid++;
                }else {
                    std::vector<node>::iterator nit = find(fg_nodes.begin(), fg_nodes.end(), new_node);
                    return fg_nodes[nit-fg_nodes.begin()];
                }
                return new_node;
            }

            void insert_edge(node &start, node &end) {
                if(edges.find(start) != edges.end()){
                    edges[start].push_back(end);
                }else {
                    std::vector<node> vn;
                    vn.push_back(end);
                    edges[start] = vn;
                }
                if(back_edges.find(end) != back_edges.end()){
                    back_edges[end].push_back(start);
                }else {
                    std::vector<node> v;
                    v.push_back(start);
                    back_edges[end] = v;
                }
                start.edges.push_back(end);
                end.back_edges.push_back(start);
            }

            void calculate_loops ( ) {
                int loop_cnt = -1;
                for(auto &idx: loop) {
                    loop_cnt++;
                    node start = idx.first;
                    node end = idx.second;
                    // Calculate blocks in a loop
                    std::stack<node> st;
                    std::set<node, cmpNode> loop_set;
                    loop_set.insert(start);
                    loop_set.insert(end);
                    st.push(end);
                    while(!st.empty()){
                        node temp = st.top();
                        st.pop();
                        std::vector<node> pred = back_edges[temp];
                        for(auto &p : pred){
                            unsigned int prevsize = loop_set.size();
                            loop_set.insert(p);
                            if(prevsize != loop_set.size()) {
                                st.push(p);
                            }
                        }
                    }
                    std::vector<node> bbs;
                    std::vector<int> node_ids;
                    for(auto &i : loop_set){
                        bbs.push_back(i);
                        node_ids.push_back(i.id);
                    }
                    loop_basic_block.push_back(bbs);
                    loop_num_nodes[loop_cnt] = node_ids;
                    loop_nodes[loop_cnt] = bbs;
                    loop_start_nodes.push_back(idx.first.id);
                    loop_set.clear();
                    bbs.clear();
                }
            }

            void print_nested_loops(){
                for(unsigned int j=0; j<loop_start_nodes.size(); j++){
                    unsigned int i = 0;
                    while(i<loop_start_nodes.size()){
                        if(i!=j && find(loop_num_nodes[i].begin(), loop_num_nodes[i].end(), loop_start_nodes[j]) != loop_num_nodes[i].end()){
                            errs() << "Loop # " << j << " is nested within loop # " << i << "\n";
                        }
                        i++;
                    }   
                }
            }

            void print_loops(){
                
            }
    };
}
namespace {
	// Function pass to analyse every function in source code 
	
	struct functionAnalysis : public FunctionPass {
		static int functioncount ;
		static char ID;
		functionAnalysis() : FunctionPass(ID) {}
		
		bool runOnFunction(Function & F) override {

			// Declaring and Initializing variables for counting
			
			errs() << "Function Name: " << F.getName() << '\n';
			functioncount++;
            int basicblock_count = 0;
            loopAnalysis::flowgraph fg;
			for (auto& B : F) {
                basicblock_count++;
                loopAnalysis::node n = fg.insert_node(&B);
                TerminatorInst* tinst = B.getTerminator(); 
                int num_succ = tinst->getNumSuccessors();
                for(int i=0; i<num_succ;i++){
                    loopAnalysis::node end = fg.insert_node(tinst->getSuccessor(i));
                    fg.insert_edge(n, end);
                }
            }
            // for(auto &n : fg.fg_nodes){
            //     errs()<<"Node #: " << n.id << "\n";
            //     errs()<<"Edge to #: ";
            //     for(auto &subnode : fg.edges[n]) {
            //         errs() << subnode.id << " - ";
            //     }
            //     errs() << "\n";
            //     errs()<<"Back Edges to #: ";
            //     for(auto &subnode : fg.back_edges[n]) {
            //         errs() << subnode.id << " - ";
            //     }
            //     errs() << "\n";
            // }
            DominatorTree DT = DominatorTree(F);
            std::vector<std::vector<int>> dom_mat;

            for(int i=0; i < fg.nodeid; i++){
                TerminatorInst *ins = fg.fg_nodes[i].bb->getTerminator();
                std::vector<int> temp2;
                for(int j=0; j< fg.nodeid; j++){
                    if(DT.dominates(ins, fg.fg_nodes[j].bb)){
                        temp2.push_back(j);
                    }
                }
                dom_mat.push_back(temp2);
                temp2.clear();
            }

            std::vector<std::vector<int>> loop_st_end_nodes;
            for(auto &kvalue: fg.edges){
                for(auto &edg : kvalue.second){
                    if (edg.id < kvalue.first.id){
                        /* Found a Backward edge */
                        if (std::find(dom_mat[edg.id].begin(), dom_mat[edg.id].end(), kvalue.first.id) != dom_mat[edg.id].end()){
                            std::vector<int> temp;
                            std::pair<loopAnalysis::node, loopAnalysis::node> temp2(edg, kvalue.first);
                            fg.loop.push_back(temp2);
                            temp.push_back(kvalue.first.id);
                            temp.push_back(edg.id);
                            loop_st_end_nodes.push_back(temp);
                            temp.clear();
                        }
                    }
                }
            }

            fg.calculate_loops();
            errs() << "Number of Loops: " << fg.loop_basic_block.size() << " \n\n";
            for(auto& vn: fg.loop_nodes){
                errs()<< "Loop #: "<<vn.first<< "\n";
                errs()<< "\t Basic Blocks #: " << vn.second.size() << "\n";
                int instcount = 0;
                for(auto &n: vn.second){
                    for(auto &I : *(n.bb)){
                        instcount++;
                    }
                }
                errs() <<"\t Instruction count #: " <<instcount << "\n";
            }
            errs()<<"\n";

            fg.print_nested_loops();
            return false;
        }
        
    };
}


char functionAnalysis::ID = 0;
int functionAnalysis::functioncount = 0;
int loopAnalysis::flowgraph::nodeid = 0;
static RegisterPass<functionAnalysis> A("functionanalysis", "Function analysis prints instruction count for different categories in a function");