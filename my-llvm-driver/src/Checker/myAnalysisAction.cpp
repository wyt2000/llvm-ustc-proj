#include "clang/Frontend/CompilerInstance.h"
#include "Checker/myAnalysisAction.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Frontend/AnalysisConsumer.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"

#include "Checker/SimpleDivZeroChecker.h"
#include "Checker/SimpleStreamChecker.h"
#include "Checker/MallocOverflowSecurityChecker.h"
#include "Checker/CStringChecker.h"
#include "Checker/NewArgChecker.h"
#include "Checker/DestructorChecker.h"
#include "Checker/DestructorStaChecker.h"
#include "Checker/LeakEvalOrderChecker.h"
#include "Checker/DestructorThrowChecker.h"
#include "Checker/DumbChecker.h"
#include "Checker/StaticInitReenteredChecker.h"
// TODO: include你编写的Checker的头文件


using namespace llvm;
using namespace clang;
using namespace clang::ento;

std::unique_ptr<ASTConsumer> myAnalysisAction::CreateASTConsumer(
  
  CompilerInstance &CI, llvm::StringRef InFile) {

  // 创建AnalysisConsumer
  std::unique_ptr<clang::ento::AnalysisASTConsumer> AnalysisConsumer = CreateAnalysisConsumer(CI);

  std::map<std::string, int> CheckerNameList = {
    {"H2020.SimpleDivChecker",                  0},
    {"H2020.SimpleStreamChecker",               1},
    {"H2020.MallocOverflowSecurityChecker",     2},
    {"H2020.CStringChecker",                    3},
    {"H2020.NewArgChecker",                     4},
    {"H2020.DestructorChecker",                 5},
    {"H2020.DestructorStaChecker",              6},
    {"H2020.LeakEvalOrderChecker",              7},
    {"H2020.DestructorThrowChecker",            8},
    {"H2020.StaticInitReenteredChecker",        9},
    {"All",                                     10}
  };

  for (auto s : CheckerArgs) {
    if(!CheckerNameList.count(s)) {
      continue;
    }
    switch (CheckerNameList[s]) {
      case 0:
        addChecker<SimpleDivChecker>(CI, AnalysisConsumer.get(), "H2020.SimpleDivChecker");
        break;
      case 1:
        addChecker<SimpleStreamChecker>(CI, AnalysisConsumer.get(), "H2020.SimpleStreamChecker");
        break;
      case 2:
        addChecker<MallocOverflowSecurityChecker>(CI, AnalysisConsumer.get(), "H2020.MallocOverflowSecurityChecker");
        break;
      case 3:
        addChecker<CStringChecker>(CI, AnalysisConsumer.get(), "H2020.CStringChecker");
        break;
      case 4:
        addChecker<DumbChecker>(CI, AnalysisConsumer.get(), "DumbChecker");
        addChecker<NewArgChecker>(CI, AnalysisConsumer.get(), "H2020.NewArgChecker");
        break;
      case 5:
        addChecker<DestructorChecker>(CI, AnalysisConsumer.get(), "H2020.DestructorChecker");
        break;
      case 6:
        addChecker<DestructorStaChecker>(CI, AnalysisConsumer.get(), "H2020.DestructorStaChecker");
        break;
      case 7:
        addChecker<LeakEvalOrderChecker>(CI, AnalysisConsumer.get(), "H2020.LeakEvalOrderChecker");
        break;
      case 8:
        addChecker<DestructorThrowChecker>(CI, AnalysisConsumer.get(), "H2020.DestructorThrowChecker");
        break;
      case 9:
        addChecker<StaticInitReenteredChecker>(CI, AnalysisConsumer.get(), "H2020.StaticInitReenteredChecker");
      case 10:
        addChecker<SimpleDivChecker>(CI, AnalysisConsumer.get(), "H2020.SimpleDivChecker");
        addChecker<SimpleStreamChecker>(CI, AnalysisConsumer.get(), "H2020.SimpleStreamChecker");
        addChecker<MallocOverflowSecurityChecker>(CI, AnalysisConsumer.get(), "H2020.MallocOverflowSecurityChecker");
        addChecker<CStringChecker>(CI, AnalysisConsumer.get(), "H2020.CStringChecker");
        addChecker<NewArgChecker>(CI, AnalysisConsumer.get(), "H2020.NewArgChecker");
        addChecker<DestructorChecker>(CI, AnalysisConsumer.get(), "H2020.DestructorChecker");
        addChecker<DestructorStaChecker>(CI, AnalysisConsumer.get(), "H2020.DestructorStaChecker");
        addChecker<LeakEvalOrderChecker>(CI, AnalysisConsumer.get(), "H2020.LeakEvalOrderChecker");
        addChecker<DestructorThrowChecker>(CI, AnalysisConsumer.get(), "H2020.DestructorThrowChecker");
        addChecker<StaticInitReenteredChecker>(CI, AnalysisConsumer.get(), "H2020.StaticInitReenteredChecker");
        break;
    }
  }
  // TODO: 在这里按照上面的示例调用addChecker函数添加自己编写的Checker


  return AnalysisConsumer;
}
