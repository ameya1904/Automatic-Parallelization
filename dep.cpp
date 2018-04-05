#include<iostream>
#include <vector>
using namespace std;

class dep_analyser{
	
	private:
	    bool dep;
	    int dim,nest;
	    vector<vector<int>> loop;
		vector<vector<int>> data;

	public:
	    dep_analyser(int x,int t, int l[][3],int d[][3]){
	        dim=x;
	        dep=false;
	        nest=t;
	        vector<int> tmp;
	        for(int i=0;i<nest;i++){
	            for (int j=0;j<3;j++){
	                tmp.push_back(l[i][j]);
	            }
	            loop.push_back(tmp);
	            tmp.clear();
	        }
	        
	        for(int i=0;i<2;i++){
	            for (int j=0;j<nest+1;j++){
	                tmp.push_back(d[i][j]);
	            }
	            data.push_back(tmp);
	            tmp.clear();
	        }
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
	    
	    bool banerjee(char choice[2]){
	        int x,lb=0,ub=0,a_pos,a_neg,b_pos,b_neg,a,b,l,u,n,a_z,b_z;
	        a_z=data[0][nest];
	        b_z=data[0][nest];
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
	        cout<<"\t"<<lb<<"\t"<<ub<<"\t"<<dep<<"\n";
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
	        for(int i=0;i<nest;i++){
	            for(int j=0;j<3;j++){
	                cout<<loop[i][j]<<"\t";
	            }
	            cout<<"\n";
	        }
	    }
	    
	    void analyse(){
	        char meta[4]={'s','l','e','g'};
	        char choice[nest];
	        for(int i=0;i<nest;i++){
	            for(int j=0;j<4;j++){
	                cout<<
	            }
	        }
	    }
};

int main(){
    int loop[2][3]={{1,10,1},{1,10,1}};
    int data[2][3]={{10,1,0},{10,1,-1}};
    dep_analyser obj(2,2,loop,data);
    obj.gcdtest();
    char choice[2];
    choice[0]='s';choice[1]='s';
    //obj.banerjee(choice);
    //obj.banerjee('l');
    //obj.banerjee('e');
    //obj.banerjee('g');
    
    return 0;
}

