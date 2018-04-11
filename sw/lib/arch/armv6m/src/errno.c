int errno;

// return the adress of errno (important for reentrant functions)
int *__errno()
{
    return &errno;
}
