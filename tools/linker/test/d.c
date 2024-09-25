int add(int a, int b);

static int number = 1;
int counter = 2;

void setx(int x, char *text){
    int y = add(x, counter++);
    text[0] = y;
    text[1] = number++;
}