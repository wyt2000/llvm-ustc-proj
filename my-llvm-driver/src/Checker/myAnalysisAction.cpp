#include "clang/Frontend/CompilerInstance.h"
#include "Checker/myAnalysisAction.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"
#include "clang/StaticAnalyzer/Frontend/AnalysisConsumer.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"

#include "Checker/SimpleDivZeroChecker.h"
#include "Checker/SimpleStreamChecker.h"
#include "Checker/MallocOverflowSecurityChecker.h"
#include "Checker/CStringChecker.h"
#include "Checker/NewZeroChecker.h"
#include "Checker/DestructorChecker.h"
// TODO: include你编写的Checker的头文件


using namespace llvm;
using namespace clang;
using namespace clang::ento;


std::unique_ptr<ASTConsumer> myAnalysisAction::CreateASTConsumer(
  CompilerInstance &CI, llvm::StringRef InFile) {
  // 创建AnalysisConsumer
  std::unique_ptr<clang::ento::AnalysisASTConsumer> AnalysisConsumer = CreateAnalysisConsumer(CI);

  
  addChecker<SimpleDivChecker>(CI, AnalysisConsumer.get(), "H2020.SimpleDivChecker");
  addChecker<SimpleStreamChecker>(CI, AnalysisConsumer.get(), "H2020.SimpleStreamChecker");
  addChecker<MallocOverflowSecurityChecker>(CI, AnalysisConsumer.get(), "H2020.MallocOverflowSecurityChecker");
  addChecker<CStringChecker>(CI, AnalysisConsumer.get(), "H2020.CStringChecker");
  addChecker<NewZeroChecker>(CI, AnalysisConsumer.get(), "H2020.NewZeroChecker");
  addChecker<DestructorChecker>(CI, AnalysisConsumer.get(), "H2020.DestructorChecker");
  // TODO: 在这里按照上面的示例调用addChecker函数添加自己编写的Checker


  return AnalysisConsumer;
}
