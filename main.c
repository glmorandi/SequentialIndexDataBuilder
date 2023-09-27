#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#define MAX_LINE_LENGTH 2048
#define MAX_APPNAME_LENGTH 50
#define MAX_APPID_LENGTH 150
#define MAX_CATEGORY_LENGTH 23
#define MAX_INSTALLS_LENGTH 15

enum LABEL
{
    APP_NAME = 0,
    APP_ID = 1,
    CATEGORY = 2,
    INSTALLS = 3,
};

struct AppInfo
{
    char appName[MAX_APPNAME_LENGTH];
    char appId[MAX_APPID_LENGTH];
    char category[MAX_CATEGORY_LENGTH];
    char installs[MAX_INSTALLS_LENGTH];
};

// Estrutura para passar dados para a thread
struct ThreadData
{
    FILE *csvFile;
    FILE *binaryFile;
};

// Estrutura para o índice parcial
struct IndexEntry
{
    char label[MAX_APPID_LENGTH]; // Nome do campo indexado (appName, appId, category, installs)
    long position;                // Posição do registro
};

// Defina uma estrutura para o índice parcial em memória
struct MemoryIndexEntry
{
    char label[MAX_APPNAME_LENGTH]; // Nome do campo indexado (appName, appId, category, installs)
    long position;                  // Posição do registro
};

// Função para criar o índice parcial
int createFileIndex(FILE * binaryFile, const char *indexFileName, enum LABEL label)
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

        case INSTALLS:
            strncpy(indexEntry.label, appInfo.installs, sizeof(indexEntry.label));
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
int createMemoryIndex(FILE * binaryFile, struct MemoryIndexEntry **memoryIndex, int *indexSize, enum LABEL label)
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

        case INSTALLS:
            strncpy((*memoryIndex)[*indexSize].label, appInfo.installs, sizeof((*memoryIndex)[*indexSize].label));
            break;
        }

        (*memoryIndex)[*indexSize].position = currentPosition;
        (*indexSize)++;
        currentPosition = ftell(binaryFile); // Obtenha a posição atual no arquivo binário
    }
    rewind(binaryFile);
    return 0;
}

// Função para processar uma linha do arquivo CSV
void processCSVLine(const char *line, struct AppInfo *appInfo)
{
    const char *delimiter = ",";
    char *token;
    int tokenCount = 0;

    // Encontre o primeiro token
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
            // Remove as aspas duplas e os sinais de mais do campo "Installs"
            char *src = token;
            char *dst = appInfo->installs;
            while (*src)
            {
                if (*src != '"' && *src != '+')
                {
                    *dst++ = *src;
                }
                src++;
            }
            *dst = '\0'; // Encerra a string com um caractere nulo
            break;
        }
        token = strtok(NULL, delimiter);
        tokenCount++;
    }
}

// Função executada por cada thread
void *threadProcessCSV(void *arg)
{
    struct ThreadData *data = (struct ThreadData *)arg;
    char line[MAX_LINE_LENGTH];
    struct AppInfo appInfo;

    while (fgets(line, sizeof(line), data->csvFile))
    {
        processCSVLine(line, &appInfo);
        fwrite(&appInfo, sizeof(struct AppInfo), 1, data->binaryFile);
    }

    return NULL;
}

// Função para inicializar threads e processar o arquivo CSV
int processCSVFileWithThreads(const char *filename, const char *binaryFilename, int numThreads)
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

    pthread_t threads[numThreads];
    struct ThreadData threadData[numThreads];

    // Inicializar threads
    for (int i = 0; i < numThreads; i++)
    {
        threadData[i].csvFile = csvFile;
        threadData[i].binaryFile = binaryFile;
        pthread_create(&threads[i], NULL, threadProcessCSV, &threadData[i]);
    }

    // Aguardar o término das threads
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    fclose(csvFile);
    fclose(binaryFile);
    return 0;
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
void createMemoryAVLIndex(FILE * binaryFile, struct AVLNode **avlIndex, enum LABEL label)
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

        case INSTALLS:
            strncpy(indexEntry.label, appInfo.installs, sizeof(indexEntry.label));
            break;
        }

        indexEntry.position = ftell(binaryFile); // Obtenha a posição atual no arquivo binário
        *avlIndex = insertAVLNode(*avlIndex, indexEntry);
    }

    rewind(binaryFile);
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

    // Para as pesquisas
    const char *appId = "com.ironwaterstudio.masks";
    const char *appName = "Pupsy";

    const int numThreads = 4; // Número de threads a serem usadas

    if (processCSVFileWithThreads(filename, binaryFilename, numThreads) == 0)
    {
        printf("Processamento do arquivo CSV concluído com sucesso.\n");
        // readAndPrintBinaryFile(binaryFilename);
    }

    FILE *bin = fopen(binaryFilename, "rb");

    // Cria índice em arquvio do appId
    if (createFileIndex(bin, firstIndex, APP_ID) == 0)
    {
        printf("Criação do primeiro índice concluído com sucesso.\n");
    }
    rewind(bin);
    // Cria índice em arquivo do appName
    if (createFileIndex(bin, secondIndex, APP_NAME) == 0)
    {
        printf("Criação do segundo índice concluído com sucesso.\n");
    }
    rewind(bin);
    // Cria índice em memória do category
    if (createMemoryIndex(bin, &thirdIndex, &indexSize, CATEGORY) == 0)
    {
        printf("Criação do terceiro índice concluído com sucesso.\n");
    }
    rewind(bin);
    // Cria índice em memória utilizando uma AVL do installs
    struct AVLNode *avlIndex = NULL;
    createMemoryAVLIndex(bin, &avlIndex, INSTALLS);

    if (avlIndex != NULL)
    {
        printf("Criação do quarto índice concluído com sucesso.\n");
    }
    rewind(bin);
    return 0;
}