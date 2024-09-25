void setx(int x, char *text);


int add(int a, int b){
    return a + b;
}

static int number = 4;

void main(){
    int c = add(10, number++);
    c = add(c, number++);

    setx(c, "Hello world");
}