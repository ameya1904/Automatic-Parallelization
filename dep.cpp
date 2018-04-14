#include "text.h"

StatementMatcher LoopMatcher = forStmt().bind("forLoop");
StatementMatcher LoopMatcher2 = forStmt().bind("forLoop");

class LoopPrinter : public MatchFinder::MatchCallback {
	private:
		ASTContext* Context;
	public :
		virtual void run(const MatchFinder::MatchResult &Result) {
			SourceManager *sm=Result.SourceManager;
			Context = Result.Context;
			struct loop_struct loop_tmp;
			string loop_cond;
			if (const ForStmt *fl = Result.Nodes.getNodeAs<clang::ForStmt>("forLoop")){ 
				FullSourceLoc FullLocation = Context->getFullLoc(fl->getLocEnd());
				if (FullLocation.isValid()) {
					//llvm::outs() << "Found call at " << FullLocation.getSpellingLineNumber()<< ":" << FullLocation.getSpellingColumnNumber() << "\n";
					loop_tmp.lower_bound=getInitValue(fl->getInit());
					loop_tmp.inc=getIncBy(fl->getInc());
					loop_tmp.upper_bound=getCondition(fl->getCond(),loop_tmp.loop_var,loop_cond);
					loop_tmp.start=fl->getLocStart();
					loop_tmp.end=fl->getLocEnd();
					loop_data.push_back(loop_tmp);
				}
			}
		}

		int getCondition(const Expr* e,string& str,string& loop_cond){
			int val;
			//FullSourceLoc FullLocation = Context->getFullLoc(e->getLocStart());
			//llvm::outs() << "Found condn at " << FullLocation.getSpellingLineNumber()<< ":" << FullLocation.getSpellingColumnNumber() << "\n";
			if(isa<BinaryOperator>(e)){
				BinaryOperator* b = (BinaryOperator*)e;
				StringRef op = b->getOpcodeStr();
				loop_cond.assign(op.str());
				
				Expr* rhs = b->getRHS();
				if(isa<IntegerLiteral>(rhs)){
					val = intEvaluate(rhs);
				}
				Expr* lhs = b->getLHS();
				if(isa<ImplicitCastExpr>(lhs)){
					ImplicitCastExpr* ice = (ImplicitCastExpr*)lhs;
					Expr* exp = ice->getSubExpr();
					if(isa<DeclRefExpr>(exp)){
						DeclRefExpr* dre = (DeclRefExpr*)exp;
						NamedDecl* nd = dre->getFoundDecl();
						str.assign(nd->getNameAsString());
					}
				}
			}
			return val;
		}

		int getIncBy(const Expr* e){
			int val;
			//FullSourceLoc FullLocation = Context->getFullLoc(e->getLocStart());
			//llvm::outs() << "Found inc at " << FullLocation.getSpellingLineNumber()<< ":" << FullLocation.getSpellingColumnNumber() << "\n";
			if(isa<BinaryOperator>(e)){
				BinaryOperator* b = (BinaryOperator*)e;
				Expr* rhs = b->getRHS();
				if(isa<BinaryOperator>(rhs)){
					BinaryOperator* b2 = (BinaryOperator*)rhs;
					Expr* b_lhs = b2->getLHS();
					Expr* b_rhs = b2->getRHS();
					if(isa<IntegerLiteral>(b_lhs)){
						return intEvaluate(b_lhs);
					}
					else if(isa<IntegerLiteral>(b_rhs)){
						return intEvaluate(b_rhs);
					}
				}
			}
			else if(isa<UnaryOperator>(e)){
				UnaryOperator* u = (UnaryOperator*)e;
				if(u->isIncrementOp()){
					val=1;
				}
				else if(u->isDecrementOp()){
					val=-1;
				}
				return val;
			}
			return UINT64_MAX;
		}

		int intEvaluate(Expr* e){
			APSInt r;
			APSInt &Result=r;
			clang::Expr::SideEffectsKind AllowSideEffects=clang::Expr::SideEffectsKind::SE_NoSideEffects;
			ASTContext& ctx=*Context;
			uint64_t Limit =UINT64_MAX;
			if(e->EvaluateAsInt(Result,ctx, AllowSideEffects)){
				int temp=Result.getLimitedValue(Limit);
				return temp;
			}
			return Limit;
		}

		int getInitValue(const Stmt* s){
			int val;
			//FullSourceLoc FullLocation = Context->getFullLoc(s->getLocStart());
			//llvm::outs() << "Found init at " << FullLocation.getSpellingLineNumber()<< ":" << FullLocation.getSpellingColumnNumber() << "\n";
			if(isa<BinaryOperator>(s)){
			    Expr* e = ((BinaryOperator*)s)->getRHS();
			    if(isa<IntegerLiteral>(e)){
			        val=intEvaluate(e);
			    }
			}
			//cout<<"aa\t"<<val<<"\n";
			return val;
		}
};

class LoopPrinter2 : public MatchFinder::MatchCallback {
	private:
		
		vector<Expr*> indices;
		vector<const Stmt*> nodes;
		vector<BinaryOperator*> if_body; 
		struct expr{
			string obj;
			int type;
		};
		vector<expr> infix;
	public :
		virtual void run(const MatchFinder::MatchResult &Result) {
			SourceManager *sm=Result.SourceManager;
			Context = Result.Context;
			struct loop_struct loop_tmp;
			if (const ForStmt *fl = Result.Nodes.getNodeAs<clang::ForStmt>("forLoop")){
				const Stmt* s = fl->getBody();
				for(Stmt::const_child_iterator i = s->child_begin();i!=s->child_end();i++){
					const Stmt* tmp = *i;
					//FullSourceLoc FullLocation = Context->getFullLoc(tmp->getLocStart());
					//llvm::outs() << "Found stmt at "<<is_it_loop(tmp->getLocStart())<<"\t" << FullLocation.getSpellingLineNumber()<< ":" <<     FullLocation.getSpellingColumnNumber() << "\n";
                    
					if(isa<BinaryOperator>(tmp)){
						stmt_handle(tmp);
						stmt_loc.push_back(make_pair(stmt_num,tmp->getLocStart()));
						stmt_num++;
					}
					else if(isa<IfStmt>(tmp)){
					    //tmp->dump();
					    
					    IfStmt* fi = (IfStmt*)tmp;
					    if_handle(fi);
					    
					    for(vector<BinaryOperator*>::iterator i=if_body.begin();i!=if_body.end();i++){
					        //(*i)->dump();
					        stmt_handle(*i);
					        stmt_loc.push_back(make_pair(stmt_num,(*i)->getLocStart()));
						    stmt_num++;
					    }
					}

				}
			}

		}
		
		void print_loc(IfStmt* loc){
		    FullSourceLoc FullLocation = Context->getFullLoc(loc->getIfLoc());
		    llvm::outs() << "Found stmt at "<<"\t" << FullLocation.getSpellingLineNumber()<< ":" <<     FullLocation.getSpellingColumnNumber() << "\t"<<loc->getIfLoc().getRawEncoding()<<"\n";
		    FullLocation = Context->getFullLoc(loc->getElseLoc());
		    llvm::outs() << "Found stmt at "<<"\t" << FullLocation.getSpellingLineNumber()<< ":" <<     FullLocation.getSpellingColumnNumber() << "\t"<<loc->getElseLoc().getRawEncoding()<<"\n";
		}
		
		void if_handle(IfStmt* fi){ 
		    if(fi->getElseLoc().getRawEncoding()==0){
		        if_locations.push_back(make_pair(fi->getIfLoc(),fi->getLocEnd()));    
		    }
		    else{
		        if_locations.push_back(make_pair(fi->getIfLoc(),fi->getElseLoc()));
	        }
		    Stmt *body= fi->getThen(),*else_=fi->getElse();
		    if(!body){
		        return;
		    }
		    if(isa<CompoundStmt>(body)){
		        CompoundStmt* cs =(CompoundStmt*)(body);
		        for(Stmt::child_iterator i=cs->child_begin();i!=cs->child_end();i++){
		            
		            if(isa<IfStmt>(*i)){
		                if_handle((IfStmt*)(*i));
		            }
		            
		            if(isa<BinaryOperator>(*i)){
		                //(*i)->dump();
		                if_body.push_back((BinaryOperator*)(*i));
		            }
		        }
		    }
		    else if(isa<BinaryOperator>(body)){
		        //body->dump();
		        if_body.push_back((BinaryOperator*)(body));
		    }
		    
		    if(!else_){
		        return;
		    }
		    if(isa<IfStmt>(else_)){
		        if_handle((IfStmt*)else_);
		    }
		    else{
		        if_locations.push_back(make_pair(else_->getLocStart(),fi->getLocEnd()));
		        if(isa<CompoundStmt>(else_)){
		        CompoundStmt* cs =(CompoundStmt*)(else_);
		        for(Stmt::child_iterator i=cs->child_begin();i!=cs->child_end();i++){
		            
		            if(isa<IfStmt>(*i)){
		                if_handle((IfStmt*)(*i));
		            }
		            
		            if(isa<BinaryOperator>(*i)){
		                //(*i)->dump();
		                if_body.push_back((BinaryOperator*)(*i));
		            }
		        }
		    }
		    }
		    
		}

		void stmt_handle(const Stmt* stmt){
			BinaryOperator* b = (BinaryOperator*)stmt;
			stmt_handler_recursive(stmt);
			for(vector<const Stmt*>::iterator i=nodes.begin();i!=nodes.end();i++){
				Expr* e = (Expr*)(*i);
				Stmt::const_child_iterator child=e->child_begin();
				if(child!=e->child_end()){
					if(isa<ArraySubscriptExpr>(*child)){
						e = (Expr*)(*child);
					}
				}
				if(!isa<IntegerLiteral>(e)){
					struct var_struct var_obj = node_handler(e,b);
					//print_node(var_obj);
					if(b->getLHS()==e){
						var_obj.is_def=true;
					}
					else{
						var_obj.is_def=false;
					}
					create_data(var_obj);
				}
			}

			nodes.clear();
		}

		void create_data(struct var_struct obj){
			bool found=false;
			if(obj.is_array){
				vector<struct array_id_struct>::iterator iter;
				for(vector<struct array_id_struct>::iterator i=array_id_obj.begin();i!=array_id_obj.end() & !found;i++){
					if(obj.var.compare(i->id)==0){
						found=true;
						iter=i;
					}
				}
				if(!found){
					struct array_id_struct tmp;
					tmp.id=obj.var;
					tmp.matrix.push_back(obj);
					array_id_obj.push_back(tmp);
				}
				else{
					iter->matrix.push_back(obj);
				}
			}
			else{
				vector<struct var_id_struct>::iterator iter;
				for(vector<struct var_id_struct>::iterator i=var_id_obj.begin();i!=var_id_obj.end() && !found;i++){
					if(i->id.compare(obj.var)==0){
						found=true;
						iter=i;
					}
				}
				if(!found){
					struct var_id_struct tmp;
					tmp.id=obj.var;
					tmp.matrix.push_back(obj);
					var_id_obj.push_back(tmp);
				}
				else{
					iter->matrix.push_back(obj);
				}
			}
		}

		void stmt_handler_recursive(const Stmt* stmt){
			if(isa<BinaryOperator>(stmt)){
				BinaryOperator* s = (BinaryOperator*)stmt;
				//s->getLHS()->dump();
				stmt_handler_recursive((Stmt*)s->getLHS());
				//s->getRHS()->dump();
				stmt_handler_recursive((Stmt*)s->getRHS());
			}
			else{
				nodes.push_back(stmt);
			}
		}

		struct var_struct node_handler(Expr* e,BinaryOperator* b){
			struct var_struct var_obj=getOuterLoops(b->getLocStart());
			var_obj.stmt_num=stmt_num;
			
			if(isa<ImplicitCastExpr>(e)){
				var_obj.is_array=false;
				Expr* name_node=e;
				if(isa<ImplicitCastExpr>(name_node)){
					Stmt::const_child_iterator i=name_node->child_begin();
					if(isa<DeclRefExpr>(*i)){
						DeclRefExpr* dre = (DeclRefExpr*)(*i);
						//cout<<dre->getFoundDecl()->getNameAsString()<<"\n";
						var_obj.var.assign(dre->getFoundDecl()->getNameAsString());
					}
				}
			}
			
			else if(isa<DeclRefExpr>(e)){
			    var_obj.is_array=false;
			    DeclRefExpr* dre = (DeclRefExpr*)(e);
				//cout<<dre->getFoundDecl()->getNameAsString()<<"\n";
				var_obj.var.assign(dre->getFoundDecl()->getNameAsString());
			}

			else{
				var_obj.is_array=true;
				Expr* name_node;
				//e->dump();
				handle(e,&name_node);
				/*for(vector<Expr*>::iterator i=indices.begin();i!=indices.end();i++){
				    (*i)->dump();
				}cout<<"\n\n";*/
				//name_node->dump();
				if(isa<ImplicitCastExpr>(name_node)){
					Stmt::const_child_iterator i=name_node->child_begin();
					if(isa<DeclRefExpr>(*i)){
						DeclRefExpr* dre = (DeclRefExpr*)(*i);
						//cout<<dre->getFoundDecl()->getNameAsString()<<"\n";
						var_obj.var.assign(dre->getFoundDecl()->getNameAsString());
					}
				}

				for(vector<Expr*>::iterator i = indices.begin();i!=indices.end();i++){
					parseExpr(*i);
					/*for(vector<expr>::iterator i=infix.begin();i!=infix.end();i++){
					    cout<<i->obj<<"\t";
					}cout<<"\n";*/
					getCoefficient(&var_obj);
					infix.clear();
				}
				reverse(var_obj.coeff.begin(),var_obj.coeff.end());

				indices.clear();
			}

			return var_obj;
		}

		struct var_struct getOuterLoops(SourceLocation loc){
			struct var_struct obj;
			obj.is_array=true;
			obj.stmt_num=stmt_num;
			vector<int> tmp;
			for(vector<loop_struct>::iterator i =loop_data.begin();i!=loop_data.end();i++){
				if(loc.getRawEncoding()>=(*i).start.getRawEncoding() && loc.getRawEncoding()<=(*i).end.getRawEncoding()){
					obj.enclosers.push_back(i->loop_var);
					tmp.push_back(i->lower_bound);
					tmp.push_back(i->upper_bound);
					tmp.push_back(i->inc);
					obj.loop.push_back(tmp);
					tmp.clear();
				}
			}
			obj.enclosers.push_back("__const__");
			return obj;
		}

		void insertAtPos(struct var_struct *obj,string s, int num){
			int j=0,index=obj->coeff.size();
			bool var=true,found=false;
			for(vector<string>::iterator i=obj->enclosers.begin();i!=obj->enclosers.end() && var;i++){
				if(i->compare(s)==0){
					var=false;
					found=true;
				}
				j++;
			}
			if(found){
				obj->coeff[index-1][j-1]=num;
			}
		}
		
		void compress(){
		
		    vector<int> erase_list;
		    int count=0;
		    for(vector<struct expr>::iterator i=infix.begin();i!=infix.end();i++){
		        if(i->type==5){
		            i->obj="+";
		            i->type=99;
		            erase_list.push_back(count);
		            if((i+1)->type==3){
		                (i+1)->type=6;
		            }
		            else if((i+1)->type==1){
		                (i+1)->obj="-"+(i+1)->obj;
		            }
		        }
		        else if(i->type==0){
		            if(i->obj.compare("-")==0){
		                i->obj="+";
		                if((i+1)->type==3){
		                    (i+1)->type=6;
		                }
		                else if((i+1)->type==1){
		                    (i+1)->obj="-"+(i+1)->obj;
		                }
		            }
		        }
		        count++;
		    }
		    for(int i=erase_list.size()-1;i>-1;i--){
		        infix.erase(infix.begin()+erase_list[i]);
		    }
		    
		    for(int i=0;i<infix.size();i++){
				cout<<"\t"<<infix[i].obj    <<"\t";}cout<<"\n";
				
			for(int i=0;i<infix.size();i++){
				cout<<"\t"<<infix[i].type       <<"\t";}cout<<"\n";
		    
			stack<struct expr> operator_stack;
			vector<struct expr> output;
			
			for (unsigned i = 0; i < infix.size(); i++) {
                if (infix[i].obj.compare("+")==0  || infix[i].obj.compare("-")==0 || infix[i].obj.compare("*")==0 || infix[i].obj.compare("*")==0){
                    while (!operator_stack.empty() && priority(operator_stack.top().obj) <= priority(infix[i].obj)) {
                        output.push_back(operator_stack.top());
                        operator_stack.pop();
                    }
                    operator_stack.push(infix[i]);
                }  else {
                    output.push_back(infix[i]);
                }
            }
            
            while (!operator_stack.empty()) {
                output.push_back(operator_stack.top());
                operator_stack.pop();
            }
            
            infix=output;
		}
        
        int priority(string a) 
        {
             int temp;
             if( a.compare("^")==0){
                 temp = 1;
             }
             else
             {
                 if(a.compare("*")==0|| a.compare("/")==0){
                      temp = 2;
                 }
                 else
                 {
                     if(a.compare("+")==0 || a.compare("-")==0)
                          temp = 3;
                 }
             }
             return temp;
        }

        
		void getCoefficient(struct var_struct *obj){
			stack<struct expr> stack_;
			string operatore;
			vector<int> coeff;
            compress();
            
			for(int i=0;i<obj->enclosers.size();i++){
				coeff.push_back(0);
			}
			obj->coeff.push_back(coeff);

			if(infix.size()==1){
				//cout<<infix[0].obj<<"\t1\n";
				if(infix[0].type==3){
				    insertAtPos(obj,infix[0].obj,1);
			    }
			    else if(infix[0].type==6){
			        insertAtPos(obj,infix[0].obj,-1);
			    }
			    else{
			        insertAtPos(obj,"__const__",stoi(infix[0].obj));
			    }
			}
			
			for(int i=0;i<infix.size();i++){
				cout<<"\t"<<infix[i].obj    <<"\t";}cout<<"\n";
				
			for(int i=0;i<infix.size();i++){
				cout<<"\t"<<infix[i].type       <<"\t";}cout<<"\n";
			
			struct expr op1,op2,tmp={"$",4};
			for(int i=0;i<infix.size();i++){
				//cout<<"\t"<<infix[i].obj    <<"\t";
				if(infix[i].type!=0 && infix[i].type!=5){
					stack_.push(infix[i]);
				}
				
				else if(infix[i].type==0){
					op1=stack_.top();
					stack_.pop();
					op2=stack_.top();
					stack_.pop();
					
					
					if(infix[i].obj.compare("*")==0){
					    if(op1.type==3){
					        if(op2.type<3){
					            insertAtPos(obj,op1.obj,stoi(op2.obj));
					        }
					        else if(op2.type==6){
					            //insertAtPos(obj,op1.obj,stoi(op2.obj));
					        }
					        else if(op2.type==7){
					            //case of -j*i
					        }
					    }
					    else if(op1.type<3){
					        if(op2.type<3){
					            int val=stoi(op2.obj)*stoi(op2.obj);
					            insertAtPos(obj,"__const__",val);
					        }
					        else if(op2.type==3){
					            insertAtPos(obj,op2.obj,stoi(op1.obj));
					        }
					        else if(op2.type==6){
					            int val=-1*stoi(op1.obj);
					            insertAtPos(obj,op2.obj,val);
					        }
					    }
					    else if(op1.type==6){
					        if(op2.type<3){
					            insertAtPos(obj,op1.obj,-1*stoi(op2.obj));
					        }
					    }
					}
					else if(infix[i].obj.compare("+")==0){
					    if(op1.type<3){
					        if(op2.type<3){
					            insertAtPos(obj,"__const__",stoi(op1.obj)+stoi(op2.obj));
					            string s="";
					            s+=stoi(op1.obj)+stoi(op2.obj);
					            tmp={s,1};
				            }
				            else if(op2.type==3) {
				                insertAtPos(obj,"__const__",stoi(op1.obj));
				                insertAtPos(obj,op2.obj,1);
				            }
				            else if(op2.type==6){
				                insertAtPos(obj,"__const__",stoi(op1.obj));
				                insertAtPos(obj,op2.obj,-1);
				            }
				            else if(op2.type==4){
				                insertAtPos(obj,"__const__",stoi(op1.obj));
				            }
					    }
					    else if(op1.type==3){
					        if(op2.type<3){
					            insertAtPos(obj,"__const__",stoi(op2.obj));
					            insertAtPos(obj,op1.obj,1);
					        }
					        else if(op2.type==3){
					            insertAtPos(obj,op1.obj,1);
					            insertAtPos(obj,op2.obj,1);
					        }
					        else if(op2.type==6){
					            insertAtPos(obj,op1.obj,1);
					            insertAtPos(obj,op2.obj,-1);
					        }
					        else if(op2.type==4){
					            insertAtPos(obj,op1.obj,1);
					        }
					    }
					    else if(op1.type==6){
					        if(op2.type<3){
					            insertAtPos(obj,"__const__",stoi(op2.obj));
					            insertAtPos(obj,op1.obj,-1);
					        }
					        else if(op2.type==3){
					            insertAtPos(obj,op1.obj,-1);
					            insertAtPos(obj,op2.obj,1);
					        }
					        else if(op2.type==6){
					            insertAtPos(obj,op1.obj,-1);
					            insertAtPos(obj,op2.obj,-1);
					        }
					        else if(op2.type==4){
					            insertAtPos(obj,op1.obj,-1);
					        }
					    }
					    else if(op1.type==4){
					        if(op2.type<3){
					            insertAtPos(obj,"__const__",stoi(op2.obj));
					        }
					        else if(op2.type==3){
					            insertAtPos(obj,op2.obj,1);
					        }
					        else if(op2.type==6){
					            insertAtPos(obj,op2.obj,-1);
					        }
					    }
					}
					stack_.push(tmp);
				}
					
					/*if((infix[i].obj.compare("*")==0 || infix[i].obj.compare("/")==0)){
						if(op1.type==3){
							//cout<<op1.obj<<"\t"<<op2.obj<<"\n";
							//cout<<"stoi\t"<<op2.obj<<"\n";
							if(op2.type==6){
							    insertAtPos(obj,op1.obj,stoi('-'+op2.obj));
							}
							else if(op2.type==){
							    
							}
							else{
							    insertAtPos(obj,op1.obj,stoi(op2.obj));
						    }
						}
						else if(op2.type==3){
							//cout<<op2.obj<<"\t"<<op1.obj<<"\n";
                            if(op1.type==6){
							    insertAtPos(obj,op2.obj,stoi('-'+op1.obj));
							}
							else{
							    insertAtPos(obj,op2.obj,stoi(op1.obj));
							}
						}
					}
					else if(infix[i].obj.compare("+")==0){
						if(op1.type<3){
							//cout<<"const coeff\t"<<op1.obj<<"\n";
							insertAtPos(obj,"__const__",stoi(op1.obj));
						}
						if(op2.type<3){
							//cout<<"const coeff\t"<<op1.obj<<"\n";
							insertAtPos(obj,"__const__",stoi(op2.obj));
						}
						if(op1.type==3){
							//cout<<op1.obj<<"\t1\n";
							insertAtPos(obj,op1.obj,1);
						}
						if(op2.type==3){
							//cout<<op2.obj<<"\t1\n";
							insertAtPos(obj,op2.obj,1);
						}
					}
					else if(infix[i].obj.compare("-")==0){
						if(op1.type<3){
							//cout<<"const coeff\t"<<op1.obj<<"\n";
							insertAtPos(obj,"__const__",stoi('-'+op1.obj));
						}
						if(op2.type<3){
							//cout<<"const coeff\t"<<op2.obj<<"\n";
							insertAtPos(obj,"__const__",stoi(op2.obj));
						}
						if(op1.type==3){
							//cout<<op1.obj<<"\t-1\n";
							insertAtPos(obj,op1.obj,-1);
						}
						if(op2.type==3){
							//cout<<op2.obj<<"\t-1\n";
							insertAtPos(obj,op2.obj,1);
						}
					}
					struct expr tmp = {"$",4};
					stack_.push(tmp);
				}*/
			}cout<<"\n";
		}

		void parseExpr(Expr* e){
			//e->dump();cout<<"\n";
			struct expr tmp;
			if(isa<BinaryOperator>(e)){
				BinaryOperator* b = (BinaryOperator*)e;
				StringRef op = b->getOpcodeStr();
				tmp.obj=op.str();
				tmp.type=0;
				parseExpr(b->getLHS());
				infix.push_back(tmp);
				parseExpr(b->getRHS());  
				 
			}
			if(isa<UnaryOperator>(e)){
				UnaryOperator* u = (UnaryOperator*)e;
				StringRef op = u->getOpcodeStr(u->getOpcode());
				
				if(op.str().compare("-")==0){
				    tmp.obj='%';
				    tmp.type=5;
				    infix.push_back(tmp);
				}
				parseExpr(u->getSubExpr());  
			}
			if(isa<IntegerLiteral>(e)){
				tmp.obj=to_string(intEvaluate(e));
				tmp.type=1;
				infix.push_back(tmp);
			}
			if(isa<ImplicitCastExpr>(e)){
				Stmt::const_child_iterator i=e->child_begin();
				if(isa<DeclRefExpr>(*i)){
					DeclRefExpr* dre = (DeclRefExpr*)(*i);
					tmp.obj=dre->getFoundDecl()->getNameAsString();
					tmp.type=3;
					infix.push_back(tmp);
				} 
			}
		}

		void handle(Expr* e,Expr** name_node){
			Stmt::const_child_iterator i;
			bool go_deep=true;
			if(isa<ArraySubscriptExpr>(e)){
				ArraySubscriptExpr* e1 = (ArraySubscriptExpr*)e;
				Expr* e2 = e1->getRHS();
				*name_node=e1->getLHS();
				//(*name_node)->dump();
				indices.push_back(e2);
				i = e1->child_begin();
				if(i==e->child_end()){
					go_deep=false;
				}
			}
			else{
				i = e->child_begin();
				if(i==e->child_end()){
					go_deep=false;
				}
			}
			if(go_deep){
				handle((Expr*)(*i),name_node);
			}
		}

		bool is_it_loop(SourceLocation s){
			bool val=false;
			for(vector<struct loop_struct>::iterator i=loop_data.begin();i!=loop_data.end();i++){
				if(s==i->start){
					val=true;
				}
			}
			return val;
		}  

		int intEvaluate(Expr* e){
			APSInt r;
			APSInt &Result=r;
			clang::Expr::SideEffectsKind AllowSideEffects=clang::Expr::SideEffectsKind::SE_NoSideEffects;
			ASTContext& ctx=*Context;
			uint64_t Limit =UINT64_MAX;
			if(e->EvaluateAsInt(Result,ctx, AllowSideEffects)){
				int temp=Result.getLimitedValue(Limit);
				return temp;
			}
			return Limit;
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

};

void print_node(struct var_struct obj){
	cout<<obj.stmt_num<<"\t"<<"Varname\t"<<obj.var<<"\n";
	for(unsigned i=0;i<obj.coeff.size();i++){
		for(unsigned j=0;j<obj.enclosers.size();j++){
			cout<<"\t"<<obj.coeff[i][j]<<"\t";
		}
		cout<<"\n";
	}
}

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
