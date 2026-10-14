
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096

int ReadPipeLine(HANDLE pipe, char *buffer, DWORD size)
{
    DWORD bytes_read;
    DWORD i = 0;
    char ch;

    while (i < size - 1)
    {
        if (!ReadFile(pipe, &ch, 1, &bytes_read, NULL))
        {
            return 0;
        }

        if (bytes_read == 0)
        {
            return 0;
        }

        buffer[i++] = ch;

        if (ch == '\n')
        {
            break;
        }
    }

    buffer[i] = '\0';
    return 1;
}

int WritePipeLine(HANDLE pipe, const char *line)
{
    DWORD bytes_written;
    DWORD length = (DWORD)strlen(line);

    if (!WriteFile(pipe, line, length, &bytes_written, NULL))
    {
        return 0;
    }

    return bytes_written == length;
}

int main(void)
{
    SECURITY_ATTRIBUTES sa;
    HANDLE inputPipe[2];
    HANDLE middlePipe[2];
    HANDLE outputPipe[2];

    STARTUPINFOA si1;
    STARTUPINFOA si2;
    PROCESS_INFORMATION pi1;
    PROCESS_INFORMATION pi2;

    char command1[] = "child1.exe";
    char command2[] = "child2.exe";
    char line[BUFFER_SIZE];
    char result[BUFFER_SIZE];

    int running = 1;

    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&inputPipe[0], &inputPipe[1], &sa, 0) ||
        !CreatePipe(&middlePipe[0], &middlePipe[1], &sa, 0) ||
        !CreatePipe(&outputPipe[0], &outputPipe[1], &sa, 0))
    {
        printf("%lu\n", GetLastError());
        return 1;
    }

    SetHandleInformation(inputPipe[1], HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(outputPipe[0], HANDLE_FLAG_INHERIT, 0);

    ZeroMemory(&si1, sizeof(si1));
    ZeroMemory(&si2, sizeof(si2));
    ZeroMemory(&pi1, sizeof(pi1));
    ZeroMemory(&pi2, sizeof(pi2));

    si1.cb = sizeof(si1);
    si1.dwFlags = STARTF_USESTDHANDLES;
    si1.hStdInput = inputPipe[0];
    si1.hStdOutput = middlePipe[1];
    si1.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    si2.cb = sizeof(si2);
    si2.dwFlags = STARTF_USESTDHANDLES;
    si2.hStdInput = middlePipe[0];
    si2.hStdOutput = outputPipe[1];
    si2.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (!CreateProcessA(
            ".\\child2.exe",
            command2,
            NULL, NULL, TRUE, 0,
            NULL, NULL, &si2, &pi2))
    {
        printf("%lu\n", GetLastError());
        return 1;
    }

    if (!CreateProcessA(
            ".\\child1.exe",
            command1,
            NULL, NULL, TRUE, 0,
            NULL, NULL, &si1, &pi1))
    {
        printf("%lu\n", GetLastError());

        TerminateProcess(pi2.hProcess, 1);
        CloseHandle(pi2.hThread);
        CloseHandle(pi2.hProcess);
        return 1;
    }

    CloseHandle(inputPipe[0]);
    CloseHandle(middlePipe[0]);
    CloseHandle(middlePipe[1]);
    CloseHandle(outputPipe[1]);

    printf("Enter a string:\n");

    while (running && fgets(line, sizeof(line), stdin) != NULL)
    {
        if (strchr(line, '\n') == NULL &&
            !feof(stdin))
        {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }

            printf("line is too long\n");
            continue;
        }

        if (strcmp(line, "exit\n") == 0 ||
            strcmp(line, "exit\r\n") == 0)
        {
            if (!WritePipeLine(inputPipe[1], "exit\n"))
            {
                printf("%lu\n", GetLastError());
            }

            if (ReadPipeLine(outputPipe[0], result, BUFFER_SIZE))
            {
                printf("%s", result);
            }

            running = 0;
            break;
        }

        if (!WritePipeLine(inputPipe[1], line))
        {
            printf("%lu\n", GetLastError());
            break;
        }

        if (!ReadPipeLine(outputPipe[0], result, BUFFER_SIZE))
        {
            printf("%lu\n", GetLastError());
            break;
        }

        printf("%s", result);
    }

    CloseHandle(inputPipe[1]);
    CloseHandle(outputPipe[0]);

    WaitForSingleObject(pi1.hProcess, INFINITE);
    WaitForSingleObject(pi2.hProcess, INFINITE);

    CloseHandle(pi1.hThread);
    CloseHandle(pi1.hProcess);
    CloseHandle(pi2.hThread);
    CloseHandle(pi2.hProcess);

    return 0;
}
