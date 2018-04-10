#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include<iostream>
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/LLVM.h"
#include<string>
#include<utility>
#include<algorithm>
#include <math.h>

using namespace std;
using namespace clang::tooling;
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;

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
	        //cout<<gcd_<<"\t"<<x<<"\t"<<dep<<"\n";
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
vector<struct edge> edge_vector;

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
            for(int i=0;i<dim;i++){
                tmp.push_back(data1[i]);
                tmp.push_back(data2[i]);
                dep_analyser obj(dim,nest,loop,tmp);
                obj.analyse();
                tmp_result[i]=obj.result;
                tmp.clear();
            }
            for(int i=0;i<dim-1;i++){
                for(vector<string>::iterator j= tmp_result[i].begin();j!=tmp_result[i].end();j++){
                    for(vector<string>::iterator k= tmp_result[i+1].begin();k!=tmp_result[i+1].end();k++){
	                    //cout<<*j<<"\t";
	                    string tmp;
	                    tmp.assign(product(*j,*k));
	                    if(is_valid(tmp)){
	                        result.push_back(tmp);
	                    }
                    }
	            }
            }
            for(vector<string>::iterator j= result.begin();j!=result.end();j++){
                cout<<*j<<"\n";
            }
        }
        
        bool is_valid(string result){
            bool val=true;
            for(int i=0;i<nest+1;i++){
                if(result[i]=='x'){
                    val=false;
                }
            }cout<<val;
            return val;
        }
        
        void analyse(){
            for(vector<struct array_id_struct>::iterator i=array_id_obj.begin();i!=array_id_obj.end();i++){
                for(vector<struct var_struct>::iterator j=i->matrix.begin();(j)!=i->matrix.end();j++){
                    for(vector<struct var_struct>::iterator k=j+1;(k)!=i->matrix.end();k++){
                        /*
                        print_node(*j);
                        print_node(*k);
                        for(int l=0;l<j->loop.size();l++){
                            for(int m=0;m<j->loop[l].size();m++){
                                cout<<j->loop[l][m]<<"\t";
                            }
                            cout<<"\n";
                        }
                        for(int l=0;l<k->loop.size();l++){
                            for(int m=0;m<k->loop[l].size();m++){
                                cout<<k->loop[l][m]<<"\t";
                            }
                            cout<<"\n";
                        }
                        cout<<"dim\t"<<j->coeff.size()<<"nest\t"<<j->enclosers.size()<<"\n";
                        */
                        struct edge tmp;
                        tmp.begin=j->stmt_num;
                        tmp.end=k->stmt_num;
                        
                        if(j->loop==k->loop){
                            cout<<"yes\n";
                            set_data(j->coeff.size(),j->enclosers.size()-1,j->loop,j->coeff,k->coeff);
                            get_dependance();
                            tmp.dep_vector=result;
                        }
                        else{
                            cout<<"no\n";
                            tmp.dep_vector.push_back("ss");
                        }
                        edge_vector.push_back(tmp);
                        
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
};
