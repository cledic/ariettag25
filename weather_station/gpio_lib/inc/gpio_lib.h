
char get_gpio_Num( int gpioid);
char get_gpio_Port( int gpioid);

int gpiosetdir(int gpioid,char *mode);
int gpiogetbits(int gpioid);
int gpiosetbits(int gpioid);
int gpioclearbits(int gpioid);

