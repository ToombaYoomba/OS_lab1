
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(void)
{
    char line[4096];
    size_t i;

    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "exit") == 0)
        {
            printf("exit\n");
            fflush(stdout);
            break;
        }

        for (i = 0; line[i] != '\0'; i++)
        {
            line[i] = (char)toupper((unsigned char)line[i]);
        }

        printf("%s\n", line);
        fflush(stdout);
    }

    return 0;
}
