#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define MAX_LINE_LENGTH 2048
#define MAX_APPNAME_LENGTH 50
#define MAX_APPID_LENGTH 150
#define MAX_CATEGORY_LENGTH 25
#define MAX_DEVID_LENGTH 50

enum LABEL
{
    APP_NAME = 0,
    APP_ID = 1,
    CATEGORY = 2,
    DEV_ID = 3,
};

struct AppInfo
{
    char appName[MAX_APPNAME_LENGTH];
    char appId[MAX_APPID_LENGTH];
    char category[MAX_CATEGORY_LENGTH];
    char devId[MAX_DEVID_LENGTH];
};

// Estrutura para o índice parcial
struct IndexEntry
{
    char label[MAX_APPID_LENGTH]; // Nome do campo indexado (appName, appId, category, devId)
    long position;                // Posição do registro
};

// Defina uma estrutura para o índice parcial em memória
struct MemoryIndexEntry
{
    char label[MAX_APPNAME_LENGTH]; // Nome do campo indexado (appName, appId, category, devId)
    long position;                  // Posição do registro
};

int processCSVFile(const char *filename, const char *binaryFilename)
{
    FILE *csvFile = fopen(filename, "r");
    if (csvFile == NULL)
    {
        perror("Erro ao abrir o arquivo CSV");
        return 1;
    }

    FILE *binaryFile = fopen(binaryFilename, "wb");
    if (binaryFile == NULL)
    {
        perror("Erro ao abrir o arquivo binário");
        fclose(csvFile);
        return 1;
    }

    FILE *textFile = fopen("teste.txt", "w");

    char line[MAX_LINE_LENGTH];
    struct AppInfo appInfo;

    const char *delimiter = ",";
    char *token;
    int tokenCount = 0;

    while (fgets(line, sizeof(line), csvFile))
    {
        // Reinicialize a estrutura AppInfo para cada linha
        memset(&appInfo, 0, sizeof(struct AppInfo));

        tokenCount = 0;
        token = strtok((char *)line, delimiter);

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
                strncpy(appInfo.devId, token, sizeof(appInfo.devId));
                break;
            }
            token = strtok(NULL, delimiter);
            tokenCount++;
        }

        fwrite(&appInfo, sizeof(struct AppInfo), 1, binaryFile);

        fprintf(textFile, "App Name: %s\n", appInfo.appName);
        fprintf(textFile, "App ID: %s\n", appInfo.appId);
        fprintf(textFile, "Category: %s\n", appInfo.category);
        fprintf(textFile, "Installs: %s\n", appInfo.devId);
        fprintf(textFile, "-------------------------\n");
    }

    fclose(csvFile);
    fclose(binaryFile);
    fclose(textFile);
    return 0;
}

// Função para criar o índice parcial
int createFileIndex(FILE *binaryFile, const char *indexFileName, enum LABEL label)
{
    if (binaryFile == NULL)
    {
        perror("Erro ao abrir o arquivo binário");
        return -1;
    }

    FILE *indexFile = fopen(indexFileName, "wb");
    if (indexFile == NULL)
    {
        perror("Erro ao abrir o arquivo de índice");
        fclose(binaryFile);
        return -1;
    }

    struct AppInfo appInfo;
    struct IndexEntry indexEntry;
    long currentPosition = 0;

    while (fread(&appInfo, sizeof(struct AppInfo), 1, binaryFile) == 1)
    {
        switch (label)
        {
        case APP_NAME:
            strncpy(indexEntry.label, appInfo.appName, sizeof(indexEntry.label));
            break;

        case APP_ID:
            strncpy(indexEntry.label, appInfo.appId, sizeof(indexEntry.label));
            break;

        case CATEGORY:
            strncpy(indexEntry.label, appInfo.category, sizeof(indexEntry.label));
            break;

        case DEV_ID:
            strncpy(indexEntry.label, appInfo.devId, sizeof(indexEntry.label));
            break;
        }
        indexEntry.position = currentPosition;
        fwrite(&indexEntry, sizeof(struct IndexEntry), 1, indexFile);
        currentPosition = ftell(binaryFile); // Obtenha a posição atual no arquivo binário
    }
    rewind(binaryFile);
    fclose(indexFile);
    return 0;
}

// Função para criar o índice parcial em memória
int createMemoryIndex(FILE *binaryFile, struct MemoryIndexEntry **memoryIndex, int *indexSize, enum LABEL label)
{
    if (binaryFile == NULL)
    {
        perror("Erro ao abrir o arquivo binário");
        return -1;
    }

    struct AppInfo appInfo;
    long currentPosition = 0;
    int indexCapacity = 1000; // Tamanho inicial do array de índices
    *memoryIndex = (struct MemoryIndexEntry *)malloc(sizeof(struct MemoryIndexEntry) * indexCapacity);

    if (*memoryIndex == NULL)
    {
        perror("Erro ao alocar memória para o índice em memória");
        fclose(binaryFile);
        return -1;
    }

    while (fread(&appInfo, sizeof(struct AppInfo), 1, binaryFile) == 1)
    {
        if (*indexSize >= indexCapacity)
        {
            // Se o array de índices estiver cheio, aumente sua capacidade
            indexCapacity *= 2;
            *memoryIndex = (struct MemoryIndexEntry *)realloc(*memoryIndex, sizeof(struct MemoryIndexEntry) * indexCapacity);

            if (*memoryIndex == NULL)
            {
                perror("Erro ao realocar memória para o índice em memória");
                fclose(binaryFile);
                return -1;
            }
        }

        switch (label)
        {
        case APP_NAME:
            strncpy((*memoryIndex)[*indexSize].label, appInfo.appName, sizeof((*memoryIndex)[*indexSize].label));
            break;

        case APP_ID:
            strncpy((*memoryIndex)[*indexSize].label, appInfo.appId, sizeof((*memoryIndex)[*indexSize].label));
            break;

        case CATEGORY:
            strncpy((*memoryIndex)[*indexSize].label, appInfo.category, sizeof((*memoryIndex)[*indexSize].label));
            break;

        case DEV_ID:
            strncpy((*memoryIndex)[*indexSize].label, appInfo.devId, sizeof((*memoryIndex)[*indexSize].label));
            break;
        }

        (*memoryIndex)[*indexSize].position = currentPosition;
        (*indexSize)++;
        currentPosition = ftell(binaryFile); // Obtenha a posição atual no arquivo binário
    }
    rewind(binaryFile);
    return 0;
}

void processCSVLine(const char *line, struct AppInfo *appInfo)
{
    const char *delimiter = ",";
    char *token;
    int tokenCount = 0;

    token = strtok((char *)line, delimiter);

    while (token != NULL && tokenCount < 4)
    {
        switch (tokenCount)
        {
        case 0:
            strncpy(appInfo->appName, token, sizeof(appInfo->appName));
            break;
        case 1:
            strncpy(appInfo->appId, token, sizeof(appInfo->appId));
            break;
        case 2:
            strncpy(appInfo->category, token, sizeof(appInfo->category));
            break;
        case 3:
            strncpy(appInfo->devId, token, sizeof(appInfo->devId));
            break;
        }
        token = strtok(NULL, delimiter);
        tokenCount++;
    }
}

// Estrutura para o nó da árvore AVL
struct AVLNode
{
    struct MemoryIndexEntry data;
    struct AVLNode *left;
    struct AVLNode *right;
    int height;
};

// Função para obter a altura de um nó
int getHeight(struct AVLNode *node)
{
    if (node == NULL)
        return 0;
    return node->height;
}

// Função para calcular o fator de balanceamento de um nó
int getBalanceFactor(struct AVLNode *node)
{
    if (node == NULL)
        return 0;
    return getHeight(node->left) - getHeight(node->right);
}

// Função para criar um novo nó na árvore AVL
struct AVLNode *createAVLNode(struct MemoryIndexEntry data)
{
    struct AVLNode *newNode = (struct AVLNode *)malloc(sizeof(struct AVLNode));
    if (newNode == NULL)
    {
        perror("Erro ao alocar memória para o nó da árvore AVL");
        return NULL;
    }
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    newNode->height = 1;
    return newNode;
}

// Função para realizar uma rotação à direita
struct AVLNode *rotateRight(struct AVLNode *y)
{
    struct AVLNode *x = y->left;
    struct AVLNode *T2 = x->right;

    // Realize a rotação
    x->right = y;
    y->left = T2;

    // Atualize as alturas
    y->height = 1 + fmax(getHeight(y->left), getHeight(y->right));
    x->height = 1 + fmax(getHeight(x->left), getHeight(x->right));

    return x;
}

// Função para realizar uma rotação à esquerda
struct AVLNode *rotateLeft(struct AVLNode *x)
{
    struct AVLNode *y = x->right;
    struct AVLNode *T2 = y->left;

    // Realize a rotação
    y->left = x;
    x->right = T2;

    // Atualize as alturas
    x->height = 1 + fmax(getHeight(x->left), getHeight(x->right));
    y->height = 1 + fmax(getHeight(y->left), getHeight(y->right));

    return y;
}

// Função para inserir um nó na árvore AVL
struct AVLNode *insertAVLNode(struct AVLNode *root, struct MemoryIndexEntry data)
{
    if (root == NULL)
        return createAVLNode(data);

    // Insira o nó conforme a ordem da árvore BST
    if (strcmp(data.label, root->data.label) < 0)
        root->left = insertAVLNode(root->left, data);
    else if (strcmp(data.label, root->data.label) > 0)
        root->right = insertAVLNode(root->right, data);
    else
    {
        // Caso o valor for igual, ignore
        return root;
    }

    // Atualize a altura deste nó ancestral
    root->height = 1 + fmax(getHeight(root->left), getHeight(root->right));

    // Obtenha o fator de balanceamento deste nó para verificar se ele se tornou desbalanceado
    int balance = getBalanceFactor(root);

    // Caso 1: Rotação à esquerda (LL)
    if (balance > 1 && strcmp(data.label, root->left->data.label) < 0)
        return rotateRight(root);

    // Caso 2: Rotação à direita (RR)
    if (balance < -1 && strcmp(data.label, root->right->data.label) > 0)
        return rotateLeft(root);

    // Caso 3: Rotação à esquerda-direita (LR)
    if (balance > 1 && strcmp(data.label, root->left->data.label) > 0)
    {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    // Caso 4: Rotação à direita-esquerda (RL)
    if (balance < -1 && strcmp(data.label, root->right->data.label) < 0)
    {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

// Função para criar o índice em memória usando uma árvore AVL
void createMemoryAVLIndex(FILE *binaryFile, struct AVLNode **avlIndex, enum LABEL label)
{
    if (binaryFile == NULL)
    {
        perror("Erro ao abrir o arquivo binário");
        return;
    }

    struct AppInfo appInfo;
    *avlIndex = NULL;

    while (fread(&appInfo, sizeof(struct AppInfo), 1, binaryFile) == 1)
    {
        struct MemoryIndexEntry indexEntry;

        switch (label)
        {
        case APP_NAME:
            strncpy(indexEntry.label, appInfo.appName, sizeof(indexEntry.label));
            break;

        case APP_ID:
            strncpy(indexEntry.label, appInfo.appId, sizeof(indexEntry.label));
            break;

        case CATEGORY:
            strncpy(indexEntry.label, appInfo.category, sizeof(indexEntry.label));
            break;

        case DEV_ID:
            strncpy(indexEntry.label, appInfo.devId, sizeof(indexEntry.label));
            break;
        }

        indexEntry.position = ftell(binaryFile); // Obtenha a posição atual no arquivo binário
        *avlIndex = insertAVLNode(*avlIndex, indexEntry);
    }

    rewind(binaryFile);
}

// Função para comparar dois índices por label
int compareIndexEntry(const void *a, const void *b)
{
    return strcmp(((struct IndexEntry *)a)->label, ((struct IndexEntry *)b)->label);
}

// Função de pesquisa binária no índice
long binarySearchIndex(struct IndexEntry *index, int numEntries, const char *key)
{
    int left = 0;
    int right = numEntries - 1;

    while (left <= right)
    {
        int mid = (left + right) / 2;
        int cmp = strcmp(index[mid].label, key);

        if (cmp == 0)
        {
            return index[mid].position; // Encontrou o registro
        }
        else if (cmp < 0)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return -1; // Registro não encontrado
}

// Função de consulta principal
int binarySearchIndexFile(const char *indexFileName, const char *dataFileName, enum LABEL label, const char *searchKey)
{
    FILE *indexFile = fopen(indexFileName, "rb");
    if (indexFile == NULL)
    {
        perror("Erro ao abrir o arquivo de índice");
        return -1;
    }

    fseek(indexFile, 0, SEEK_END);
    long numEntries = ftell(indexFile) / sizeof(struct IndexEntry);
    rewind(indexFile);

    struct IndexEntry *index = (struct IndexEntry *)malloc(numEntries * sizeof(struct IndexEntry));
    if (index == NULL)
    {
        perror("Erro ao alocar memória para o índice");
        fclose(indexFile);
        return -1;
    }

    fread(index, sizeof(struct IndexEntry), numEntries, indexFile);
    fclose(indexFile);

    qsort(index, numEntries, sizeof(struct IndexEntry), compareIndexEntry);

    long position = binarySearchIndex(index, numEntries, searchKey);

    if (position != -1)
    {
        FILE *dataFile = fopen(dataFileName, "rb");
        if (dataFile == NULL)
        {
            perror("Erro ao abrir o arquivo de dados");
            free(index);
            return -1;
        }

        fseek(dataFile, position, SEEK_SET);
        struct AppInfo appInfo;
        fread(&appInfo, sizeof(struct AppInfo), 1, dataFile);

        printf("\nRegistro encontrado:\n");
        printf("AppName: %s\n", appInfo.appName);
        printf("AppId: %s\n", appInfo.appId);
        printf("Category: %s\n", appInfo.category);
        printf("Developer ID: %s\n", appInfo.devId);

        fclose(dataFile);
    }
    else
    {
        printf("Registro não encontrado.\n");
    }

    free(index);

    return 0;
}

// Função de pesquisa binária em memória
void binarySearchIndexMemory(struct MemoryIndexEntry *memoryIndex, int indexSize, FILE *binaryFile, const char *target)
{
    int left = 0;
    int right = indexSize - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(memoryIndex[mid].label, target);

        if (cmp == 0)
        {
            // Encontrou o valor, retorne a posição
            long position = memoryIndex[mid].position;

            // Use o comando "seek" para acessar o registro correspondente no arquivo de dados
            struct AppInfo appInfo;
            if (fseek(binaryFile, position, SEEK_SET) != 0)
            {
                perror("Erro ao mover o ponteiro de arquivo");
                return;
            }

            if (fread(&appInfo, sizeof(struct AppInfo), 1, binaryFile) != 1)
            {
                perror("Erro ao ler o registro do arquivo");
                return;
            }

            printf("\nRegistro encontrado:\n");
            printf("AppName: %s\n", appInfo.appName);
            printf("AppId: %s\n", appInfo.appId);
            printf("Category: %s\n", appInfo.category);
            printf("Developer ID: %s\n", appInfo.devId);

            return;
        }
        else if (cmp < 0)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
}

char *removeNewline(const char *str)
{
    int len = strlen(str);
    char *cleanedStr = (char *)malloc(len + 1);

    if (cleanedStr == NULL)
    {
        return NULL;
    }

    int j = 0;

    for (int i = 0; i < len; i++)
    {
        if (str[i] != '\n')
        {
            cleanedStr[j++] = str[i];
        }
    }

    cleanedStr[j] = '\0';

    return cleanedStr;
}

// Função para realizar pesquisa binária na árvore AVL de índice e buscar no arquivo de dados
void binarySearchIndexAVL(struct AVLNode *root, FILE *binaryFile, const char *target)
{
    if (root == NULL)
        return;
    target = removeNewline(target);
    char *label = removeNewline(root->data.label);

    int compare = strcmp(target, label);

    if (compare == 0)
    {
        if (binaryFile == NULL)
        {
            perror("Erro ao abrir o arquivo binário");
            return;
        }

        long position = root->data.position - sizeof(struct AppInfo);

        if (fseek(binaryFile, position, SEEK_SET) == 0)
        {
            struct AppInfo appInfo;
            if (fread(&appInfo, sizeof(struct AppInfo), 1, binaryFile) == 1)
            {
                printf("\nRegistro encontrado:\n");
                printf("AppName: %s\n", appInfo.appName);
                printf("AppId: %s\n", appInfo.appId);
                printf("Category: %s\n", appInfo.category);
                printf("Developer ID: %s\n", appInfo.devId);
            }
            else
            {
                perror("Erro ao ler dados do arquivo binário");
            }
        }
        else
        {
            perror("Erro ao buscar posição no arquivo binário");
        }
    }
    else if (compare < 0)
    {
        binarySearchIndexAVL(root->left, binaryFile, target);
    }
    else
    {
        binarySearchIndexAVL(root->right, binaryFile, target);
    }
}

int binarySearchDataFile(const char *dataFileName, enum LABEL label, const char *key)
{
    FILE *dataFile = fopen(dataFileName, "rb");
    if (dataFile == NULL)
    {
        perror("Erro ao abrir o arquivo de dados");
        return -1;
    }

    struct AppInfo appInfo;
    long position = 0;
    int found = 0;

    while (fread(&appInfo, sizeof(struct AppInfo), 1, dataFile) == 1)
    {
        const char *comp = NULL;
        switch (label)
        {
        case APP_NAME:
            comp = appInfo.appName;
            break;
        case APP_ID:
            comp = appInfo.appId;
            break;
        case CATEGORY:
            comp = appInfo.category;
            break;
        case DEV_ID:
            comp = appInfo.devId;
            break;
        }
        if (comp != NULL && strcmp(comp, key) == 0)
        {
            found = 1;
            break;
        }

        position += sizeof(struct AppInfo);
    }

    fclose(dataFile);

    if (found)
    {
        printf("\nRegistro encontrado:\n");
        printf("AppName: %s\n", appInfo.appName);
        printf("AppId: %s\n", appInfo.appId);
        printf("Category: %s\n", appInfo.category);
        printf("Developer ID: %s\n", appInfo.devId);
    }
    else
    {
        printf("Registro não encontrado.\n");
    }

    return 0;
}

int main()
{
    // Define os nomes dos arquivos
    const char *filename = "base.csv";
    const char *binaryFilename = "file.bin";
    const char *firstIndex = "firstIndex.bin";
    const char *secondIndex = "secondIndex.bin";

    // Definições para o índice em memória
    struct MemoryIndexEntry *thirdIndex = NULL;
    int indexSize = 0;

    // Definição para o índice em AVL
    struct AVLNode *avlIndex = NULL;

    // Para as pesquisas
    const char *appId = "com.ironwaterstudio.masks";
    const char *appName = "Pupsy";
    const char *category = "Tools";
    const char *devId = "ivanGjurovic";

    if (processCSVFile(filename, binaryFilename) == 0)
        printf("Processamento do arquivo CSV concluído com sucesso.\n");

    FILE *bin = fopen(binaryFilename, "rb");

    // Cria índice em arquvio do appId
    if (createFileIndex(bin, firstIndex, APP_ID) == 0)
        printf("Criação do primeiro índice concluído com sucesso.\n");

    // Cria índice em arquivo do appName
    if (createFileIndex(bin, secondIndex, APP_NAME) == 0)
        printf("Criação do segundo índice concluído com sucesso.\n");

    // Cria índice em memória do category
    if (createMemoryIndex(bin, &thirdIndex, &indexSize, CATEGORY) == 0)
        printf("Criação do terceiro índice concluído com sucesso.\n");

    // Cria índice em memória utilizando uma AVL do devId
    createMemoryAVLIndex(bin, &avlIndex, DEV_ID);

    if (avlIndex != NULL)
        printf("Criação do quarto índice concluído com sucesso.\n");

    // Realizando as buscas

    // Primeira busca utilizando o índice em arquivo
    printf("Realizando primeira busca para campo appId: %s\n", appId);
    binarySearchIndexFile(firstIndex, binaryFilename, APP_ID, appId);

    // Segunda busca utilizando o índice em arquivo
    printf("Realizando segunda busca para campo appName: %s\n", appName);
    binarySearchIndexFile(secondIndex, binaryFilename, APP_NAME, appName);

    // Terceira busca utilizando o índice em memória
    printf("Realizando terceira busca para campo category: %s\n", category);
    binarySearchIndexMemory(thirdIndex, indexSize, bin, category);

    // Quarta busca utilizando o índice em uma AVL
    printf("Realizando quarta busca para campo devId: %s\n", devId);
    binarySearchIndexAVL(avlIndex, bin, devId);

    fclose(bin);

    // Quinta busca utilizando o arquivo binário original sem índices
    printf("Realizando quinta busca para campo appId: %s no arquivo binário:\n", devId);
    binarySearchDataFile(binaryFilename, APP_ID, appId);

    free(thirdIndex);
    free(avlIndex);

    return 0;
}