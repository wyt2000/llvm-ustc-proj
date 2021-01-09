# Clang Static Analyzer 回答问题


### 2.4 阅读[`clang/lib/StaticAnalyzer/Checkers/SimpleStreamChecker.cpp`](https://github.com/llvm/llvm-project/tree/llvmorg-11.0.0/clang/lib/StaticAnalyzer/Checkers/SimpleStreamChecker.cpp)

回答下面问题：

1. 这个 checker 对于什么对象保存了哪些状态？保存在哪里？

   - 保存文件流符号（stream symbols）的状态。

   - 包括文件是打开还是关闭，保存到  StreamState 类的 K 属性中。

   - 还保存了符号索引 SymbolRef 到 StreamState 的映射，保存到 StreamMap 中。

2. 状态在哪些时候会发生变化？

   - 在 checkPostCall 时如果发现有 fopen，则在 StreamMap 中添加返回值 FileDesc 到一个类型为 Opened 的 StreamState 的映射。

   - 在 checkPreCall 时如果发现有 fclose，则在 StreamMap 中修改返回值 FileDesc 的映射对象为一个类型为 Closed 的 StreamState 。

3. 在哪些地方有对状态的检查？

   - 在 checkPreCall 时如果发现有 fclose，则检查该 FileDesc 对应的 StreamState 是否已经被 close 了。
   - 在 checkDeadSymbols 时检查该生存期已经结束的符号对应的文件是否关闭，即资源是否泄露。

4. 函数`SimpleStreamChecker::checkPointerEscape`的逻辑是怎样的？实现了什么功能？用在什么地方？

   - 首先调用  guaranteedNotToCloseFile 判断函数调用 Call 是否会导致文件关闭，如果不会，则不需要继续处理。否则默认这些传递到函数的符号已经在某个地方被关闭了，将它们从 StreamMap 中删去。
   - 实现了当被追踪的文件指针由于函数调用而 escape 的时候，不在子函数中继续追踪它。
   - 用在函数调用的时候。

5. 根据以上认识，你认为这个简单的checker能够识别出怎样的bug？又有哪些局限性？请给出测试程序及相关的说明。

   - 能够识别出文件重复关闭和文件指针变量生存期结束时文件未关闭导致资源泄漏的 bug。

   - 处理 pointer escape 时不再追踪已经打开的文件指针，而是默认它们关闭。

     测试程序：

     ```c
     #include <stdio.h>
     
     FILE *open(char *file)
     {
         return fopen(file, "r");
     }
     
     void f1(FILE *f)
     {
         // do something...
         exit(0);
     }
     
     int main()
     {
         FILE *f = open("foo");
         f1(f);
         return 0;
     }
     ```

     该程序不会警告，因为在调用 f1(f) 的时候在 f1 的状态集合中删掉了 f，导致 f1 结束时不会检查 f 指向的文件是否关闭，导致资源泄漏。

   - 变量生存期结束时才检查是否有没有关闭的文件，无法处理全局变量重复赋值导致的资源泄露。

     测试程序：

     ```C
     #include <stdio.h>
     FILE* f;
     FILE *open(char *file)
     {
         return fopen(file, "r");
     }
     
     int main()
     {
         f = open("foo");
         f = open("bar");
         fclose(f);
         return 0;
     }
     ```

     文件 foo 没有关闭，导致了资源泄漏，但是该程序不会警告。

### 问题2-5 阅读这一节，以及必要的相关源码，回答下面的问题：

1. 增加一个 `checker` 需要增加哪些文件？需要对哪些文件进行修改？

    - 增加: 
    需要增加 `checker` 的 `cpp` 源代码文件,并对这个 `checker` 进行注册。
    - 修改: 
    需要对 `include/clang/StaticAnalyzer/Checkers/Checkers.td` 进行修改，添加对应的 `class` 和 `record`。

2. 解释 `clang_tablegen` 函数的作用。

    - 通过阅读[llvm官方文档](https://llvm.org/docs/CommandGuide/tblgen.html)可知，`clang_tablegen` 把目标描述文件(.td)翻译成 `C++` 代码或者其他格式。

    ```
    clang_tablegen(Checkers.inc -gen-clang-sa-checkers
        SOURCE Checkers.td
        TARGET ClangSACheckers)
    ```
    - 在 `CMakeLists.txt` 文件中，`Checkers.inc` 代表输出的结果文件，参数 `-gen-clang-sa-checkers` 指出用来生成 `clang` 静态分析器，`SOURCE Checkers.td` 指出输入为目标描述文件，`TARGET ClangSACheckers` 指出目标为clang静态分析器 `ClangSACheckers`。


3. `.td` 文件的作用是什么？它是怎样生成 `C++` 头文件或源文件的？这个机制有什么好处？

    - `.td` 文件的作用： 
    通过阅读[llvm官方文档](https://llvm.org/docs/TableGen/ProgRef.html)可知，`td(target descriptor)`中通过 `abstract record` 和 `concrete record` 描述了目标机器代码的简要信息。其中， `abstract record` 抽象了若干组 `concrete record`，使得 `.td` 文件更加简化。

    - 如何生成 `C++` 头文件或源文件: 
    调用`clang_tablegen`命令，解析 `.td` 文件中的 `concrete record` 并生成输出的 `C++` 文件或者其他格式的文件。在此过程中，首先进行预处理，得到扩展的 `class` 和 `record`；然后通过作用域相关的后端(`domain specific backend`)，得到`.inc`输出文件。

    - 好处: 
    通过更简单的源文件，在生成复杂的 `llvm` 后端文件，因此可以减少重复代码,从而更加方便地构造作用域相关信息(`domain specific information`)。


