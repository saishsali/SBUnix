int pow(int num, int exponent) {
    if (exponent == 0)
        return 1;

    return num * pow(num, exponent - 1);
}

int oct_to_dec(int num) {
    int oct = 0, i = 0;
    while (num > 0) {
        oct = oct + pow(8, i) * (num % 10);
        num = num / 10;
        i++;
    }

    return oct;
}

int atoi(char *str) {
    int num = 0, i;

    for (i = 0; str[i] != '\0'; i++) {
        num = num * 10 + str[i] - '0';
    }

    return num;
}
