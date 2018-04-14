#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <iostream>
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/LLVM.h"
#include <string>
#include <utility>
#include <algorithm>
#include <math.h>

using namespace std;
using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

ASTContext* Context;
static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...");
ASTContext* context;
static int stmt_num=0;
struct loop_struct{
	string loop_var;
	int lower_bound;
	int upper_bound;
	int inc;
	SourceLocation start;
	SourceLocation end;
};
vector<loop_struct> loop_data;
struct var_struct{
	int stmt_num;
	bool is_array;
	bool is_def;
	vector<string> enclosers;
	vector<vector<int>> loop;
	vector<vector<int>> coeff;
	string var;
};
struct array_id_struct{
	string id;
	vector<struct var_struct> matrix;
};
vector<struct array_id_struct> array_id_obj;
struct var_id_struct{
	string id;
	vector<struct var_struct> matrix;
};
vector<struct var_id_struct>var_id_obj;
vector<pair<int,SourceLocation>> stmt_loc;
vector<pair<SourceLocation,SourceLocation>> if_locations;
class dep_analyser{
	
	private:
	    bool dep;
	    int dim,nest,true_level;
	    vector<vector<int>> loop;
		vector<vector<int>> data;
		char meta[3];

	public:
	    vector<string> result;
	    dep_analyser(int x,int t, vector<vector<int>> l, vector<vector<int>> d){
	        dim=x;
	        dep=false;
	        nest=t;
	        vector<int> tmp;
	        meta[0]='l';meta[1]='e';meta[2]='g';
	        loop=l;
	        data=d;
	        true_level=-1;
	    }
	    
	    int pos(int x){
            if(x<0)
               return 0;   
            return x;
        }

        int neg(int x){
            if(x>0)
                return 0;
            return x;
        }
        
        int gcd(int n1,int n2){
            if (n1<0){
                n1=-n1;
            }
            if (n2<0){
                n2=-n2;
            }
            while(n1 != n2){
                if(n1 > n2)
                    n1 -= n2;
                else
                    n2 -= n1;
            }
            return n1;
        }
	    
	    bool banerjee(string choice){
	        int x,lb=0,ub=0,a_pos,a_neg,b_pos,b_neg,a,b,l,u,n,a_z,b_z;
	        a_z=data[0][nest];
	        b_z=data[1][nest];
	        x=b_z-a_z;
	        for(int i=0;i<nest;i++){
                a=data[0][i];
                b=data[1][i];
                
                l=loop[i][0];
                u=loop[i][1];
                n=loop[i][2];
                
                a_pos=this->pos(a);
                a_neg=this->neg(a);
                b_pos=this->pos(b);
                b_neg=this->neg(b);
                
                switch(choice[i]){
                    case 'l':
                        //cout<<"<\n";
                        lb+=this->neg(a_neg-b)*(u-l-n)+(a-b)*l-b*n;
                        ub+=this->pos(a_pos-b)*(u-l-n)+(a-b)*l-b*n;
                        break;
                        
                    case 'e':
                        //cout<<"=\n";
                        lb+=this->neg(a-b)*(u-l)+(a-b)*l;
                        ub+=this->pos(a-b)*(u-l)+(a-b)*l;
                        break;
                
                    case 'g':
                        //cout<<">\n";
                        lb+=this->neg(a-b_pos)*(u-l-n)+(a-b)*l+a*n;
                        ub+=this->pos(a-b_neg)*(u-l-n)+(a-b)*l+a*n;
                        break;
                        
                    case 's':
                        //cout<<"*\n";
                        lb+=(a_neg-b_pos)*(u-l)+(a-b)*l;
                        ub+=(a_pos-b_neg)*(u-l)+(a-b)*l;
                        break;
                      
                    default:
                        cout<<"Invalid choice\n";
                }
                
	        }
	        if(x>=lb && x<=ub){
	            dep=true;
	        }
	        else{
	            dep=false;
	        }
	        cout<<lb<<"\t"<<x<<"\t"<<ub<<"\t"<<dep<<"\n";
	        return dep;
	    }
	    
	    bool gcdtest(){
	        int x,gcd_=data[0][0],a_z,b_z;
	        a_z=data[0][nest];
	        b_z=data[0][nest];
	        x=b_z-a_z;
	        if(x<0){
	            x=-x;
	        }
	        
	        for (int i=0;i<2;i++){
	            for (int j=0;j<nest;j++){
	                gcd_=gcd(gcd_,data[i][j]);
	            }
	        }
	        
	        if(x % gcd_==0){
	            dep=true;
	        }
	        else{
	            dep=false;
	        }
	        cout<<gcd_<<"\t"<<x<<"\t"<<dep<<"\n";
	        return dep;
	    }
	    
	    void print(){
	        for(vector<string>::iterator i= result.begin();i!=result.end();i++){
	            cout<<*i<<"\t";
	        }
	    }
	    
	    void analyse(){
	        string choice="";
	        for(int i=0;i<nest;i++){
	            choice+='s';
	        }
	        cout<<"Choice\tLB\tX\tUB\tBool\n"<<choice<<"\t";
	        if(this->banerjee(choice)){
	            analyse_recursive(1,choice);
	        }   
	        
	        /*for(vector<string>::iterator i= result.begin();i!=result.end();i++){
	            cout<<*i<<"\t";
	        }*/
	    }
	    
	    bool analyse_recursive(int level,string choice){
	        if(level>nest){
	            return false;
	        }
	        if(data[0][level-1]==0 && data[1][level-1]==0){
	            analyse_recursive(level+1,choice);
	            return false;
	        }
	        string backup;
	        backup.assign(choice);
	        for(int i=0;i<3;i++){
	            choice[level-1]=meta[i];
	            cout<<choice<<"\t";
	            if(this->banerjee(choice)){
	                if(level>true_level){
	                    true_level=level;
	                    result.clear();
	                }
	                analyse_recursive(level+1,choice);
                    if(level==true_level){
                        string tmp="";
                        tmp.assign(choice);
                        result.push_back(tmp);
                    }   
	            }
	            choice.assign(backup);
	        }
	    }
};

struct edge{
    vector<string> dep_vector;
    int begin,end;
};
struct edge2{
    string dep_vector;
    int begin,end;
    int type;
};
vector<struct edge> edge_vector;
vector<struct edge2> final_edge;
class dependance_framework{
    private:
        int nest,dim;
        vector<vector<int>> loop;
		vector<vector<int>> data1;
		vector<vector<int>> data2;
		vector<string> result;
    public:
        void set_data(int x, int y,vector<vector<int>> l,vector<vector<int>> d1, vector<vector<int>> d2){
            result.clear();
            nest=y;
            dim=x;
            loop=l;
            data1=d1;
            data2=d2;
        }
        
        void print_node(struct var_struct obj){
	        cout<<obj.stmt_num<<"\t"<<"Varname\t"<<obj.var<<"\n";
	        for(unsigned i=0;i<obj.coeff.size();i++){
		        for(unsigned j=0;j<obj.enclosers.size();j++){
			        cout<<"\t"<<obj.coeff[i][j]<<"\t";
		        }
		        cout<<"\n";
	        }
        }
        
        void get_dependance(){
            vector<vector<int>> tmp;
            vector<string> tmp_result[dim];
            string init="";
            for(int i=0;i<nest;i++){
	            init+='s';
	        }
            for(int i=0;i<dim;i++){
                tmp.push_back(data1[i]);
                tmp.push_back(data2[i]);
                dep_analyser obj(dim,nest,loop,tmp);
                obj.analyse();
                tmp_result[i]=obj.result;
                tmp.clear();
            }
            if(dim==1){
                result=tmp_result[0];
                return;
            }
            for(int i=0;i<dim-1;i++){
                for(vector<string>::iterator j= tmp_result[i].begin();j!=tmp_result[i].end();j++){
                    for(vector<string>::iterator k= tmp_result[i+1].begin();k!=tmp_result[i+1].end();k++){
                        if(tmp_result[i+1].size()<1){
                            //cout<<"qwewq"<<*j;
                            result.push_back(*j);
                            continue;
                        }
	                    //cout<<"asd\t"<<*j<<"\t"<<*k<<"\n";
	                    string tmp;
	                    tmp.assign(product(*j,*k));
	                    if(is_valid(tmp)){
	                        result.push_back(tmp);
	                    }
                    }
	            }
            }
            /*for(vector<string>::iterator j= result.begin();j!=result.end();j++){
                cout<<"sss"<<*j<<"\n";
            }*/
        }
        
        bool is_valid(string result){
            bool val=true;
            for(int i=0;i<nest+1;i++){
                if(result[i]=='x'){
                    val=false;
                }
            }//cout<<val;
            return val;
        }
        
        void analyse(){
            for(vector<struct array_id_struct>::iterator i=array_id_obj.begin();i!=array_id_obj.end();i++){
                for(vector<struct var_struct>::iterator j=i->matrix.begin();(j)!=i->matrix.end();j++){
                    for(vector<struct var_struct>::iterator k=j+1;(k)!=i->matrix.end();k++){
                        print_node(*j);cout<<"\n";print_node(*k);cout<<"is_def\t"<<j->is_def<<"\t"<<k->is_def<<"\n";
                        struct edge2 tmp;
                        if(!j->is_def && !k->is_def){
                            continue;
                        }
                        if(j->loop==k->loop){
                            //cout<<"yes\n";
                            set_data(j->coeff.size(),j->enclosers.size()-1,j->loop,j->coeff,k->coeff);
                            get_dependance();
                            vector<struct edge2> eo_vector= eo_calc(j->stmt_num,k->stmt_num);
                            
                            for(vector<string>::iterator m=result.begin();m!=result.end();m++){
                                cout<<"dep\t"<<*m<<"\n";
                                bool found=false;
                                for(vector<struct edge2>::iterator n=eo_vector.begin();n!=eo_vector.end() && !found;n++){
                                    cout<<"exec\t"<<n->begin<<"\t"<<n->end<<"\t"<<n->dep_vector<<"\n";
                                    string tmp2;
                                    tmp2.assign(product(*m,n->dep_vector));
                                    cout<<"dep x exec\t"<<tmp2<<"\t"<<is_valid(tmp2)<<"\n";
                                    /*if(j->is_def){
                                        if(j->stmt_num!=k->stmt_num){
                                            if(k->is_def){
                                                tmp.type=2;
                                            }
                                            else{
                                                tmp.type=0;
                                            }
                                        }
                                        else{
                                            tmp.type=1;
                                        }
                                    }
                                    else{
                                        if(k->is_def){
                                            tmp.type=1;
                                        }
                                    }*/
                                    if(j->stmt_num==k->stmt_num){
                                        tmp.type=1;
                                    }
                                    else if(j->stmt_num<k->stmt_num){
                                        if(j->is_def && k->is_def){
                                            tmp.type=2;
                                        }
                                        else if(j->is_def && !k->is_def){
                                            tmp.type=0;
                                        }
                                        else if(!j->is_def && k->is_def){
                                            tmp.type=1;
                                        }
                                    }
                                    else{
                                        if(j->is_def && k->is_def){
                                            tmp.type=2;
                                        }
                                        else if(j->is_def && !k->is_def){
                                            tmp.type=1;
                                        }
                                        else if(!j->is_def && k->is_def){
                                            tmp.type=0;
                                        }
                                    }
                                    if(is_valid(tmp2)){
                                        found=true;
                                        tmp.begin=n->begin;
                                        tmp.end=n->end;
                                        tmp.dep_vector=tmp2;
                                        final_edge.push_back(tmp);
                                    }
                                }
                            }
                            
                        }
                        else{
                            //cout<<"no\n";
                            string def="";
                            for(int i=0;i<nest;i++){
                                def+='s';
                            }
                            tmp.dep_vector=def;
                            final_edge.push_back(tmp);
                        }
                        
                        
                    }
                }
            }
        }
        
        string product(string s1, string s2){
            string result="";
            for(int i=0;i<nest+1;i++){
                //cout<<s1[i]<<"\t"<<s2[i]<<"\n";
                if(s1[i]=='s'){
                    result+=s2[i];
                }
                else if(s2[i]=='s'){
                    result+=s1[i];
                }
                else{
                    if(s1[i]==s2[i]){
                        result+=s1[i];
                    }
                    else{
                        result+="x";
                    }
                }
            }
            return result;
        }
        
        vector<struct edge2> eo_calc(int n1,int n2){
            int found=0;
            SourceLocation loc1,loc2;
		    vector<string> exec_vec;
            for(vector<pair<int,SourceLocation>>::iterator i=stmt_loc.begin();i!=stmt_loc.end() && found!=2;i++){
                if(i->first==n1){
                    loc1=i->second;
                    found++;
                }
                if(i->first==n2){
                    loc2=i->second;
                    found++;
                }
            }
            
            if(found!=2){
                cout<<"Error:Invalid Statement Number\n";
                //return;
            }
            
            vector<int> q1,q2;
            //cout<<"qwe\t"<<n1<<"\t"<<n2<<"\n";print_loc(loc1);print_loc(loc2);
            for(vector<pair<SourceLocation,SourceLocation>>::iterator i=if_locations.begin();i!=if_locations.end();i++){
                //print_loc(i->first);print_loc(i->second);
                if(loc1.getRawEncoding()>=i->first.getRawEncoding() && loc1.getRawEncoding()<=i->second.getRawEncoding()){
                    q1.push_back(1);
                }
                else{
                    q1.push_back(0);
                }
                
                if(loc2.getRawEncoding()>=i->first.getRawEncoding() && loc2.getRawEncoding()<=i->second.getRawEncoding()){
                    q2.push_back(1);
                }
                else{
                    q2.push_back(0);
                }
            }
            
            string tmp="";
            //cout<<"dim\t"<<dim;
            for(int i=0;i<nest;i++){
                tmp+='e';
            }
            vector<struct edge2> eo_vector;
            struct edge2 tmp2;
            if(q1==q2){
                //cout<<"1111\n";
                
                /*if(n1==n2){
                    tmp[0]='l';
                    for(int i=1;i<nest;i++){
                        tmp[i]='s';
                    }
                    tmp2.dep_vector=tmp;
                    tmp2.begin=n1;
                    tmp2.end=n2;
                    eo_vector.push_back(tmp2);
                    tmp2.begin=n2;
                    tmp2.end=n1;
                    eo_vector.push_back(tmp2);
                    //cout<<"\t"<<tmp<<"\n";
                }
                else{*/
                    if(n1<n2){
                        tmp2.dep_vector=tmp;
                        tmp2.begin=n1;
                        tmp2.end=n2;
                        eo_vector.push_back(tmp2);
                    }
                    else{
                        tmp2.dep_vector=tmp;
                        tmp2.begin=n2;
                        tmp2.end=n1;
                        eo_vector.push_back(tmp2);
                    }
                    for(int i=nest-1;i>-1;i--){
                        
                        for(int j=i;j<nest;j++){
                            tmp[j]='s';
                        }
                        tmp[i]='l';
                        tmp2.dep_vector=tmp;
                        tmp2.begin=n1;
                        tmp2.end=n2;
                        eo_vector.push_back(tmp2);
                        tmp2.begin=n2;
                        tmp2.end=n1;
                        eo_vector.push_back(tmp2);
                        //cout<<"\t"<<tmp<<"\n";
                    }
                //}
            }
            else{
                //cout<<"0000\n";
                
                if(n1==n2){
                    tmp[0]='l';
                    for(int i=1;i<nest;i++){
                        tmp[i]='s';
                    }
                    tmp2.dep_vector=tmp;
                    tmp2.begin=n1;
                    tmp2.end=n2;
                    eo_vector.push_back(tmp2);
                    tmp2.begin=n2;
                    tmp2.end=n1;
                    eo_vector.push_back(tmp2);
                    //cout<<"\t"<<tmp<<"\n";
                }
                else{
                    
                    for(int i=nest-1;i>-1;i--){
                        
                        for(int j=i;j<nest;j++){
                            tmp[j]='s';
                        }
                        tmp[i]='l';
                        tmp2.dep_vector=tmp;
                        tmp2.begin=n1;
                        tmp2.end=n2;
                        eo_vector.push_back(tmp2);
                        tmp2.begin=n2;
                        tmp2.end=n1;
                        eo_vector.push_back(tmp2);
                        //cout<<"\t"<<tmp<<"\n";
                    }
                    
                }
            }
            return eo_vector;
        }
        
        void print_loc(SourceLocation loc){
		    FullSourceLoc FullLocation = Context->getFullLoc(loc);
		    llvm::outs() << "Found stmt at "<<"\t" << FullLocation.getSpellingLineNumber()<< ":" <<     FullLocation.getSpellingColumnNumber() << "\n";
		}
		
};

