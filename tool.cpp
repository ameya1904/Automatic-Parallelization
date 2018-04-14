#include "framework.h"

int main(int argc, const char **argv) {
        CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
        ClangTool Tool(OptionsParser.getCompilations(),OptionsParser.getSourcePathList());
        
        LoopPrinter Printer;
        LoopPrinter2 Printer2;
        
        MatchFinder Finder,Finder2;
        
        Finder.addMatcher(LoopMatcher, &Printer);
        Finder2.addMatcher(LoopMatcher2, &Printer2);
        
        Tool.run(newFrontendActionFactory(&Finder).get());
        Tool.run(newFrontendActionFactory(&Finder2).get());
        
        dependance_framework obj;
        
        for(int i=0;i<array_id_obj.size();i++){
          cout<<"--------"<<array_id_obj[i].id<<"---------"<<"\n";
          for(int j=0;j<array_id_obj[i].matrix.size();j++){
            print_node(array_id_obj[i].matrix[j]);
          }
        }
        for(vector<loop_struct>::iterator i=loop_data.begin();i!=loop_data.end();i++){
            cout<<i->lower_bound<<"\t"<<i->upper_bound<<"\t"<<i->inc<<"\n";
        }
        obj.analyse();cout<<"\n";
        for(vector<struct edge2>::iterator i=final_edge.begin();i!=final_edge.end();i++){
            cout<<i->begin<<"\t"<<i->end<<"\t"<<i->dep_vector<<"\t"<<i->type<<"\n";
        }
          /*for(int i=0;i<var_id_obj.size();i++){
          cout<<"--------"<<var_id_obj[i].id<<"---------"<<"\n";
          for(int j=0;j<var_id_obj[i].matrix.size();j++){
          print_node(var_id_obj[i].matrix[j]);
          }
          }*/
        //Tool.run(newFrontendActionFactory<FindNamedClassAction>().get());
        /*
           for(vector<struct loop_struct>::iterator i=loop_data.begin();i!=loop_data.end();i++){
           cout<<i->loop_var<<"\t"<<i->lower_bound<<"\t"<<i->upper_bound<<"\t"<<i->inc<<"\n";  
           }
         */
        return 0;
}
