int test(int i);

int foo(int j){
    return test(j);
}

int test(int i)
{
    static int s = foo(i);// warn
    return 2;
}
