#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 2048

#define MAX_APPNAME_LENGTH 50
#define MAX_APPID_LENGTH 150
#define MAX_CATEGORY_LENGTH 23
#define MAX_INSTALLS_LENGTH 15

struct AppInfo
{
    char appName[MAX_APPNAME_LENGTH];   // Nome do aplicativo
    char appId[MAX_APPID_LENGTH];       // ID do aplicativo
    char category[MAX_CATEGORY_LENGTH]; // Categoria do aplicativo
    char installs[MAX_INSTALLS_LENGTH]; // Número de instalações do aplicativo
};

// Função para processar um arquivo CSV e escrever os dados parseados em um arquivo binário
int processCSVFile(const char *csvFileName, const char *binaryFileName)
{
    FILE *csvFile = fopen(csvFileName, "r");
    if (csvFile == NULL)
    {
        perror("Erro ao abrir o arquivo CSV");
        return 1; // Código de erro indicando falha
    }

    FILE *binaryFile = fopen(binaryFileName, "wb");
    if (binaryFile == NULL)
    {
        perror("Erro ao abrir o arquivo binário");
        fclose(csvFile);
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    struct AppInfo appInfo;

    while (fgets(line, sizeof(line), csvFile))
    {
        char *token;
        int tokenCount = 0;

        char *saveptr = NULL;
        token = strtok_r(line, ",", &saveptr);

        while (token != NULL && tokenCount < 4)
        {
            switch (tokenCount)
            {
            case 0:
                strncpy(appInfo.appName, token, sizeof(appInfo.appName));
                break;
            case 1:
                strncpy(appInfo.appId, token, sizeof(appInfo.appId));
                break;
            case 2:
                strncpy(appInfo.category, token, sizeof(appInfo.category));
                break;
            case 3:
                // Remove as aspas duplas e os sinais de mais do campo "Installs"
                int len = strlen(token);
                int j = 0;
                for (int i = 0; i < len; i++)
                {
                    if (token[i] != '"' && token[i] != '+')
                    {
                        appInfo.installs[j++] = token[i];
                    }
                }
                appInfo.installs[j] = '\0'; // Encerra a string com um caractere nulo
                break;
            }
            token = strtok_r(NULL, ",", &saveptr);
            tokenCount++;
        }

        fwrite(&appInfo, sizeof(struct AppInfo), 1, binaryFile);
    }

    fclose(csvFile);
    fclose(binaryFile);
    return 0; // Sucesso
}

// Função para ler e imprimir o conteúdo de um arquivo binário
void readAndPrintBinaryFile(const char *binaryFileName)
{
    FILE *binaryFile = fopen(binaryFileName, "rb");
    if (binaryFile == NULL)
    {
        perror("Erro ao abrir o arquivo binário");
        return;
    }

    struct AppInfo appInfo;

    while (fread(&appInfo, sizeof(struct AppInfo), 1, binaryFile) == 1)
    {
        printf("Nome do Aplicativo: %s\n", appInfo.appName);
        printf("ID do Aplicativo: %s\n", appInfo.appId);
        printf("Categoria: %s\n", appInfo.category);
        printf("Instalações: %s\n", appInfo.installs);
        printf("\n");
    }

    fclose(binaryFile);
}

int main()
{
    // Lembrar de alterar o nome do arquivo CSV para "base.csv" para testes antes da entrega
    const char *filename = "mini.csv";   // Nome do arquivo CSV de entrada
    const char *binaryFile = "file.bin"; // Nome do arquivo binário de saída
    if (processCSVFile(filename, binaryFile) == 0)
    {
        readAndPrintBinaryFile(binaryFile);
    }
    return 0;
}
