int main(){
    int num;
    for(int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            num = 0;
            if(j < 5) {
                goto end;
            }
        }
    }
    end:
    return 0;
}