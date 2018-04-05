#include<iostream>
#include <vector>
#include <math.h>
#include<string>
using namespace std;

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
	        string choice="ss";
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

class dependance_framework{
    private:
        int nest,dim;
        vector<vector<int>> loop;
		vector<vector<int>> data1;
		vector<vector<int>> data2;
		vector<string> result_tmp;
		string result;
    public:
        dependance_framework(int x, int y,vector<vector<int>> l,vector<vector<int>> d1, vector<vector<int>> d2){
            nest=y;
            dim=x;
            loop=l;
            data1=d1;
            data2=d2;
            result="ss";
        }
        void get_dependance(){
            vector<vector<int>> tmp;
            for(int i=0;i<dim;i++){
                tmp.push_back(data1[i]);
                tmp.push_back(data2[i]);
                dep_analyser obj(dim,nest,loop,tmp);
                obj.analyse();
                obj.print();
                
                tmp.clear();
            }
        }
        
        void product(string s1, string s2){
            for(int i=0;i<nest+1;i++){
                cout<<s1[i]<<"\t"<<s2[i]<<"\n";
            }
        }
};

int main(){
    /*vector<vector<int>> loop{{1,10,1},{1,10,1}};
    vector<vector<int>> data{{0,01,0},{0,01,01}};
    dep_analyser obj(2,2,loop,data);
    //obj.gcdtest();  
    //obj.banerjee(choice);
    //obj.banerjee('l');
    //obj.banerjee('e');
    //obj.banerjee('g');
    obj.analyse();obj.print();*/
    vector<vector<int>> loop{{1,10,1},{1,10,1}};
    vector<vector<int>> data1{{1,0,1},{0,1,0}};
    vector<vector<int>> data2{{1,0,0},{0,1,1}};
    dependance_framework obj(2,2,loop,data1,data2);
    obj.get_dependance();
    return 0;
}

