
#include <stdio.h>
#include <string.h>

int main(void)
{
    char line[4096];
    size_t i;
    size_t j;
    int previous_space;

    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "exit") == 0)
        {
            printf("exit\n");
            fflush(stdout);
            break;
        }

        i = 0;
        j = 0;
        previous_space = 0;

        while (line[i] != '\0')
        {
            if (line[i] == ' ')
            {
                if (!previous_space)
                {
                    line[j] = line[i];
                    j++;
                }

                previous_space = 1;
            }
            else
            {
                line[j] = line[i];
                j++;
                previous_space = 0;
            }

            i++;
        }

        line[j] = '\0';

        printf("%s\n", line);
        fflush(stdout);
    }

    return 0;
}
