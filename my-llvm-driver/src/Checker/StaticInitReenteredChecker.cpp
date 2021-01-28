#include "Checker/StaticInitReenteredChecker.h"
#include <iostream>
using namespace clang; 
using namespace ento;
#define WHITE 0
#define GRAY 1
#define BLACK 2

// global function call graph
std::vector<FunctionState> FunctionCallGraph;
// marks the current function to check
int current = -1;
// whether f->g exists
bool call_flag = false;

void DFS_visit(int start,int end,std::vector<int> &colors){
  colors[start] = GRAY;
  int degree = FunctionCallGraph[start].callees.size();
  for(int j = 0; j < degree;++j){
    int v = FunctionCallGraph[start].callees[j];
    if(v == end)
      call_flag = true;
    else if(colors[v] == WHITE){
        DFS_visit(v,end,colors);
    }
  }
  colors[start] = BLACK;
}

// use DFS in FunctionCallGraph to check whether
// f calls g
void DFS(std::string f,std::string g){
  call_flag = false;
  int vex_num = FunctionCallGraph.size();
  std::vector<int> colors;
  colors.resize(vex_num);
  for(int i = 0;i < vex_num;++i){
    colors[i] = WHITE;
  }

  // get the vertex of start and end
  int start,end;
  for(int i = 0;i < vex_num;++i){
    if(FunctionCallGraph[i].name == f){
      start = i;
      break;
    }
  }
  
  for(int i = 0;i < vex_num;++i){
    if(FunctionCallGraph[i].name == g){
      end = i;
      break;
    }
  }
  DFS_visit(start,end,colors);
}


// check static variable declaration(suppose in funtion f).
void StaticInitReenteredChecker::checkPreStmt(const DeclStmt *DS, 
                    CheckerContext &C) const {
  //
  ProgramStateRef state = C.getState();

  for (const auto *I : DS->decls()) {
    const VarDecl *VD = dyn_cast<VarDecl>(I);
    if (!VD)
      continue;
    // we only focus on static variables
    if(!VD->isStaticLocal())
      continue;
    // the init value to the variable
    // we only care about the function call
    const Expr* init = VD->getAnyInitializer();
    const CallExpr* CE = dyn_cast<CallExpr>(init);
    if(!CE)
      continue;
    // check the arguments of the assigning function 

    // get the function declaration of that function
    // check whether there is f in that function
    const FunctionDecl* FD = CE->getDirectCallee();
    if (!FD) 
        continue;
    
    // get the current function name
    const auto *LCtx = C.getLocationContext();
    const auto *cur_FD = dyn_cast<FunctionDecl>(LCtx->getDecl());
    if(!cur_FD)
      return;
    std::string cur_func_name = cur_FD->getNameAsString();
    
    // check the assigning function
    std::string func_name = FD->getNameAsString();
    // whether the assigning function will call the current function
    DFS(func_name,cur_func_name);
    if(call_flag){
      reportBug("recusive declaration on static variable",C);
    }
    // check the arguments
    int arg_num = CE->getNumArgs();
    for(int i = 0;i < arg_num;++i){
      const Expr *arg = CE->getArg(i);
      const CallExpr* arg_ce = dyn_cast<CallExpr>(arg);
      if(!arg_ce)
        continue;
      std::string arg_ce_name = arg_ce->getDirectCallee()->getNameAsString();
      DFS(arg_ce_name,cur_func_name);
      if(call_flag){
        reportBug("recusive declaration on static variable",C);
      }
    }
  }
}

// check the call relation at first
void StaticInitReenteredChecker::checkBeginFunction(CheckerContext &C) const {
  // get function declaration
  const auto *LCtx = C.getLocationContext();
  const auto *FD = dyn_cast<FunctionDecl>(LCtx->getDecl());
  if (!FD){
    return;
  }
  std::string func_name = FD->getNameInfo().getAsString();
  // try to find the function in the graph
  int FCG_size = FunctionCallGraph.size();
  int i = 0;
  for(;i < FCG_size;++i){
    if(FunctionCallGraph[i].name == func_name)
      break;
  }

  // the function hasn't been found,add the vertex
  if( i == FCG_size ){
    FunctionState FS;
    FS.name = func_name;
    FS.parent = current;
    FunctionCallGraph.push_back(FS);
    // add the edge
    if(current >= 0)
      FunctionCallGraph[current].callees.
            push_back(FunctionCallGraph.size() - 1);
    // update current  
    current = FunctionCallGraph.size() - 1;
  }
  // the function has been found
  else{
    // add the edge if possible
    int degree = FunctionCallGraph[current].callees.size();
    int j = 0;
    for(;j < degree;j++){
      int out = FunctionCallGraph[current].callees[j];
      if(FunctionCallGraph[out].name == func_name){
        break;
      }
    }
    if(j == degree){
      FunctionCallGraph[current].callees.push_back(i);
    }
  }
}

// check the call relation at first
void StaticInitReenteredChecker::checkEndFunction(const ReturnStmt *RS,
                                      CheckerContext &C) const {
  // get function declaration
  const auto *LCtx = C.getLocationContext();
  const auto *FD = dyn_cast<FunctionDecl>(LCtx->getDecl());
  if (!FD)
    return;
  // stop checking that function
  std::string func_name = FD->getNameInfo().getAsString();
  if(FunctionCallGraph[current].name == func_name){
    current = FunctionCallGraph[current].parent;
  }
}

void StaticInitReenteredChecker::reportBug(const char *Msg,
              CheckerContext &C) const {
  if (ExplodedNode *N = C.generateErrorNode()) {
    if (!BT)
      BT.reset(new BuiltinBug(this, "recusive declaration on static variable"));
    auto R = std::make_unique<PathSensitiveBugReport>(*BT, Msg, N);
    C.emitReport(std::move(R));
  }
}
