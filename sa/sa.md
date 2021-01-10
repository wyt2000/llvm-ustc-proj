1. 简要说明`test.c`、`AST.svg`、`CFG.svg`和`ExplodedGraph.svg`之间的联系与区别

   test.c 是程序的源文件，里面包含了一个 for 循环和一个 goto 跳转语句。

   ```
   int main() {
       L1:
       for(int i=0;i<10;i++){
           i++;
       }
       int j = 0;
       goto L1;
       return 0;
   }
   ```

   `AST.svg`、`CFG.svg`和`ExplodedGraph.svg`都是根据`test.c`作出的图，但特点和功能各有不同。

   `AST.svg`主要关注各个模块的类型特性，比如语句的类型、运算符的类型、表达式的类型等等，是用树中的父节点和子节点的形式表达从属关系的，比如 `DeclStmt`的子节点是`IntegerLiteral`，表示使用定义语句定义了一个整型变量。

   `CFG.svg`是流图。每个节点是一个基本块，用箭头表示前驱后继关系，会比较直观的表现代码内容。但其中的代码也不是`test.c`中的，而是变量会先储存左值，再把它转换成右值，用`[Bi.j]`的形式引用变量，立即数也是。

   `ExplodedGraph.svg`是一个流程图，它蕴藏了很多信息，每一个ExplodedNode中都有state_id，ProgramPoint和ProgramState，在路径敏感的数据流分析过程中对于得到的每一份数据流分析结果都要维护沿途的“路径信息”，在 path-sensitive analysis的分析中，常常使用 ExplodedGraph来表示分析结果。因为是路径敏感的，所以会和路径不敏感的CFG有所不同。

   它们中的变量从左值到右值都有一个显式变换。

2. Checker 对于程序的分析主要在 AST 上还是在 CFG 上进行？

   CFG

3. Checker 在分析程序时需要记录程序状态，这些状态一般保存在哪里？

   在 ExplodedGraph 中的 ExploedeNode 里，它包括 ProgramPoint 和 ProgramState。

4. 简要解释分析器在分析下面程序片段时的过程，在过程中产生了哪些symbolic values? 它们的关系是什么？

   ```
   int x = 3, y = 4;
   int *p = &x;
   int z = *(p + 1);
   ```

   x的值$0，y的值$1，p和&x的值$2，p+1的值$3，z和*(p+1)的值$4

   $2是$0的地址，$3是$2的自增加1，$4是$3地址位置的值。