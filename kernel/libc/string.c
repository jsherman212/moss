char *strcpy(char *dest, const char *src){
    char *src0 = (char *)src;
    while((*dest++ = *src0++));
    *dest = '\0';
    /* Who cares about strcpy return value */
    return dest;
}
