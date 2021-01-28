#!/bin/bash

./build/mClang ./tests/Test01-Malloc.cpp        -analyzer-checker=H2020.MallocOverflowSecurityChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test02-Malloc.cpp        -analyzer-checker=H2020.MallocOverflowSecurityChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test03-New.cpp           -analyzer-checker=H2020.NewArgChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test04-String.cpp        -analyzer-checker=H2020.CStringChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test05-String.cpp        -analyzer-checker=H2020.CStringChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test06-Destructor.cpp    -analyzer-checker=H2020.DestructorChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test07-Destructor.cpp    -analyzer-checker=H2020.DestructorStaChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test08-Destructor.cpp    -analyzer-checker=H2020.DestructorThrowChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test09-LeakEval.cpp      -analyzer-checker=H2020.LeakEvalOrderChecker
echo "==================================================================================================================================="
./build/mClang ./tests/Test10-StaticInit.cpp    -analyzer-checker=H2020.StaticInitReenteredChecker
