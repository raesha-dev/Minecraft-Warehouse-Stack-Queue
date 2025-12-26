/*
 * ╔════════════════════════════════════════════════════════════════╗
 * ║     COLD-CHAIN INVENTORY SIMULATOR - Interactive Console       ║
 * ║            Data Structures: Stack & Queue Implementation       ║
 * ╚════════════════════════════════════════════════════════════════╝
 *
 * Compile: gcc cold_chain_simulator.c -o simulator
 * Run: ./simulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ═══════════════════════════════════════════════════════════════════
// CONSTANTS & CONFIGURATION
// ═══════════════════════════════════════════════════════════════════

#define MAX_PRODUCTS 20
#define MAX_WAREHOUSES 5
#define MAX_BATCHES 100
#define MAX_BOXES 50
#define MAX_TRUCKS 10
#define MAX_OPERATIONS 50
#define MAX_RETURNS 20
#define MAX_NAME_LEN 50

// ═══════════════════════════════════════════════════════════════════
// DATA STRUCTURES
// ═══════════════════════════════════════════════════════════════════

typedef struct
{
    int id;
    char name[MAX_NAME_LEN];
    int minTemp;
    int maxTemp;
    int shelfLifeDays;
} Product;

typedef struct
{
    int id;
    char name[MAX_NAME_LEN];
    int currentTemp;
    int capacity;
} Warehouse;

// Queue Node for Batches (FIFO - First Expiry First Out)
typedef struct
{
    int id;
    int productId;
    int warehouseId;
    int quantity;
    char expiryDate[20];
    int daysUntilExpiry;
} Batch;

// Batch Queue Structure
typedef struct
{
    Batch items[MAX_BATCHES];
    int front;
    int rear;
    int count;
} BatchQueue;

// Box for Truck Loading
typedef struct
{
    int id;
    int batchId;
    int productId;
    int quantity;
} Box;

// Stack Structure for Truck Loading (LIFO)
typedef struct
{
    Box items[MAX_BOXES];
    int top;
} BoxStack;

typedef struct
{
    int id;
    char destination[MAX_NAME_LEN];
    BoxStack load;
    int status; // 0: idle, 1: loading, 2: on-road, 3: delivered
} Truck;

// Operation Stack for Undo functionality
typedef struct
{
    char description[100];
    int type; // 1: add_batch, 2: dispatch, 3: add_product, etc.
    int relatedId;
} Operation;

typedef struct
{
    Operation items[MAX_OPERATIONS];
    int top;
} OperationStack;

typedef struct
{
    int id;
    int productId;
    int quantity;
    char reason[MAX_NAME_LEN];
    int status; // 0: pending, 1: processed
} ReturnedItem;

// ═══════════════════════════════════════════════════════════════════
// GLOBAL STATE
// ═══════════════════════════════════════════════════════════════════

Product products[MAX_PRODUCTS];
int productCount = 0;

Warehouse warehouses[MAX_WAREHOUSES];
int warehouseCount = 0;

BatchQueue batchQueues[MAX_WAREHOUSES]; // One queue per warehouse

Truck trucks[MAX_TRUCKS];
int truckCount = 0;

OperationStack opStack;

ReturnedItem returns[MAX_RETURNS];
int returnCount = 0;

int selectedWarehouse = 0;
int nextBatchId = 1;
int nextBoxId = 1;
int nextTruckId = 1;

// ═══════════════════════════════════════════════════════════════════
// UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════
void calculateExpiryDate(char *expiryDate, int daysToAdd)
{
    time_t now = time(NULL);
    struct tm expiry = *localtime(&now);

    expiry.tm_mday += daysToAdd;
    mktime(&expiry); // normalizes date

    strftime(expiryDate, 20, "%Y-%m-%d", &expiry);
}

void clearScreen()
{
#ifdef _WIN32
    system("cls"); // Clear console on Windows
#else
    system("clear");
#endif
} // Clear console on my mac/linux. Just so we don't have clutter. To avoid scrolling.

void pauseScreen()
{
    printf("\n");
    printf("    ╔═══════════════════════════════════╗\n");
    printf("    ║   Press ENTER to continue...      ║\n");
    printf("    ╚═══════════════════════════════════╝\n");
    getchar();
    getchar();
}

void printLine(char c, int length)
{
    for (int i = 0; i < length; i++)
        printf("%c", c);
    printf("\n");
}

void printBoxTop(int width)
{
    printf("    ╔");
    for (int i = 0; i < width; i++)
        printf("═");
    printf("╗\n");
}

void printBoxBottom(int width)
{
    printf("    ╚");
    for (int i = 0; i < width; i++)
        printf("═");
    printf("╝\n");
}

void printBoxMiddle(int width)
{
    printf("    ╠");
    for (int i = 0; i < width; i++)
        printf("═");
    printf("╣\n");
}

void printBoxRow(const char *text, int width)
{
    int len = strlen(text);
    int padding = width - len;
    printf("    ║ %s", text);
    for (int i = 0; i < padding - 1; i++)
        printf(" ");
    printf("║\n");
}

void printCenteredRow(const char *text, int width)
{
    int len = strlen(text);
    int leftPad = (width - len) / 2;
    int rightPad = width - len - leftPad;
    printf("    ║");
    for (int i = 0; i < leftPad; i++)
        printf(" ");
    printf("%s", text);
    for (int i = 0; i < rightPad; i++)
        printf(" ");
    printf("║\n");
}

void printHeader(const char *title)
{
    clearScreen();
    printf("\n");
    printBoxTop(56);
    printCenteredRow("COLD-CHAIN INVENTORY SIMULATOR", 56);
    printBoxMiddle(56);
    printCenteredRow(title, 56);
    printBoxBottom(56);
    printf("\n");
}

void printMinecraftBanner()
{
    printf("\n");
    printf("    ███╗   ███╗██╗███╗   ██╗███████╗ ██████╗██████╗  █████╗ ███████╗████████╗\n");
    printf("    ████╗ ████║██║████╗  ██║██╔════╝██╔════╝██╔══██╗██╔══██╗██╔════╝╚══██╔══╝\n");
    printf("    ██╔████╔██║██║██╔██╗ ██║█████╗  ██║     ██████╔╝███████║█████╗     ██║   \n");
    printf("    ██║╚██╔╝██║██║██║╚██╗██║██╔══╝  ██║     ██╔══██╗██╔══██║██╔══╝     ██║   \n");
    printf("    ██║ ╚═╝ ██║██║██║ ╚████║███████╗╚██████╗██║  ██║██║  ██║██║        ██║   \n");
    printf("    ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝        ╚═╝   \n");
    printf("\n");
    printf("    ██████╗ ██████╗ ██╗     ██████╗      ██████╗██╗  ██╗ █████╗ ██╗███╗   ██╗\n");
    printf("   ██╔════╝██╔═══██╗██║     ██╔══██╗    ██╔════╝██║  ██║██╔══██╗██║████╗  ██║\n");
    printf("   ██║     ██║   ██║██║     ██║  ██║    ██║     ███████║███████║██║██╔██╗ ██║\n");
    printf("   ██║     ██║   ██║██║     ██║  ██║    ██║     ██╔══██║██╔══██║██║██║╚██╗██║\n");
    printf("   ╚██████╗╚██████╔╝███████╗██████╔╝    ╚██████╗██║  ██║██║  ██║██║██║ ╚████║\n");
    printf("    ╚═════╝ ╚═════╝ ╚══════╝╚═════╝      ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝\n");
    printf("\n");
    printf("        ██╗███╗   ██╗██╗   ██╗███████╗███╗   ██╗████████╗ ██████╗ ██████╗ ██╗   ██╗\n");
    printf("        ██║████╗  ██║██║   ██║██╔════╝████╗  ██║╚══██╔══╝██╔═══██╗██╔══██╗╚██╗ ██╔╝\n");
    printf("        ██║██╔██╗ ██║██║   ██║█████╗  ██╔██╗ ██║   ██║   ██║   ██║██████╔╝ ╚████╔╝ \n");
    printf("        ██║██║╚██╗██║╚██╗ ██╔╝██╔══╝  ██║╚██╗██║   ██║   ██║   ██║██╔══██╗  ╚██╔╝  \n");
    printf("        ██║██║ ╚████║ ╚████╔╝ ███████╗██║ ╚████║   ██║   ╚██████╔╝██║  ██║   ██║   \n");
    printf("        ╚═╝╚═╝  ╚═══╝  ╚═══╝  ╚══════╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝   \n");
    printf("\n");
    printf("                    ███████╗██╗███╗   ███╗██╗   ██╗██╗      █████╗ ████████╗ ██████╗ ██████╗ \n");
    printf("                    ██╔════╝██║████╗ ████║██║   ██║██║     ██╔══██╗╚══██╔══╝██╔═══██╗██╔══██╗\n");
    printf("                    ███████╗██║██╔████╔██║██║   ██║██║     ███████║   ██║   ██║   ██║██████╔╝\n");
    printf("                    ╚════██║██║██║╚██╔╝██║██║   ██║██║     ██╔══██║   ██║   ██║   ██║██╔══██╗\n");
    printf("                    ███████║██║██║ ╚═╝ ██║╚██████╔╝███████╗██║  ██║   ██║   ╚██████╔╝██║  ██║\n");
    printf("                    ╚══════╝╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚═╝  ╚═╝\n");
    printf("\n");
}

// ═══════════════════════════════════════════════════════════════════
// QUEUE OPERATIONS (FIFO - For Batches)
// ═══════════════════════════════════════════════════════════════════

void initQueue(BatchQueue *q)
{
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

int isQueueEmpty(BatchQueue *q)
{
    return q->count == 0;
}

int isQueueFull(BatchQueue *q)
{
    return q->count == MAX_BATCHES;
}

void enqueue(BatchQueue *q, Batch batch)
{
    if (isQueueFull(q))
    {
        printf("    [!] Queue is full! Cannot add batch.\n");
        return;
    }
    q->rear = (q->rear + 1) % MAX_BATCHES;
    q->items[q->rear] = batch;
    q->count++;
}

Batch dequeue(BatchQueue *q)
{
    Batch empty = {0};
    if (isQueueEmpty(q))
    {
        printf("    [!] Queue is empty! Nothing to remove.\n");
        return empty;
    }
    Batch item = q->items[q->front];
    q->front = (q->front + 1) % MAX_BATCHES;
    q->count--;
    return item;
}

Batch peekQueue(BatchQueue *q)
{
    Batch empty = {0};
    if (isQueueEmpty(q))
        return empty;
    return q->items[q->front];
}

// ═══════════════════════════════════════════════════════════════════
// STACK OPERATIONS (LIFO - For Truck Loading & Operations)
// ═══════════════════════════════════════════════════════════════════

void initBoxStack(BoxStack *s)
{
    s->top = -1;
}

int isStackEmpty(BoxStack *s)
{
    return s->top == -1;
}

int isStackFull(BoxStack *s)
{
    return s->top == MAX_BOXES - 1;
}

void pushBox(BoxStack *s, Box box)
{
    if (isStackFull(s))
    {
        printf("    [!] Stack is full! Cannot load more boxes.\n");
        return;
    }
    s->items[++s->top] = box;
}

Box popBox(BoxStack *s)
{
    Box empty = {0};
    if (isStackEmpty(s))
    {
        printf("    [!] Stack is empty! Nothing to unload.\n");
        return empty;
    }
    return s->items[s->top--];
}

Box peekBox(BoxStack *s)
{
    Box empty = {0};
    if (isStackEmpty(s))
        return empty;
    return s->items[s->top];
}

// Operation Stack
void initOpStack()
{
    opStack.top = -1;
}

void pushOperation(const char *desc, int type, int relatedId)
{
    if (opStack.top >= MAX_OPERATIONS - 1)
        return;
    opStack.top++;
    strcpy(opStack.items[opStack.top].description, desc);
    opStack.items[opStack.top].type = type;
    opStack.items[opStack.top].relatedId = relatedId;
}

Operation popOperation()
{
    Operation empty = {"", 0, 0};
    if (opStack.top == -1)
        return empty;
    return opStack.items[opStack.top--];
}

void addWarehouse();
void removeWarehouse();

// ═══════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════

void initializeData()
{
    // Initialize operation stack
    initOpStack();

    // Add sample products
    strcpy(products[0].name, "Fresh Milk");
    products[0].id = 101;
    products[0].minTemp = 2;
    products[0].maxTemp = 6;
    products[0].shelfLifeDays = 7;

    strcpy(products[1].name, "Ice Cream");
    products[1].id = 102;
    products[1].minTemp = -25;
    products[1].maxTemp = -18;
    products[1].shelfLifeDays = 180;

    strcpy(products[2].name, "Yogurt");
    products[2].id = 103;
    products[2].minTemp = 2;
    products[2].maxTemp = 8;
    products[2].shelfLifeDays = 21;

    strcpy(products[3].name, "Frozen Peas");
    products[3].id = 104;
    products[3].minTemp = -20;
    products[3].maxTemp = -15;
    products[3].shelfLifeDays = 365;

    strcpy(products[4].name, "Fresh Juice");
    products[4].id = 105;
    products[4].minTemp = 1;
    products[4].maxTemp = 5;
    products[4].shelfLifeDays = 14;

    productCount = 5;

    // Add sample warehouses
    strcpy(warehouses[0].name, "Mumbai Central");
    warehouses[0].id = 1;
    warehouses[0].currentTemp = 4;
    warehouses[0].capacity = 1000;

    strcpy(warehouses[1].name, "Pune North");
    warehouses[1].id = 2;
    warehouses[1].currentTemp = -20;
    warehouses[1].capacity = 800;

    strcpy(warehouses[2].name, "Delhi Hub");
    warehouses[2].id = 3;
    warehouses[2].currentTemp = 3;
    warehouses[2].capacity = 1200;

    warehouseCount = 3;

    // Initialize batch queues for each warehouse
    for (int i = 0; i < MAX_WAREHOUSES; i++)
    {
        initQueue(&batchQueues[i]);
    }

    // Add sample batches to warehouse 0 (Mumbai)
    Batch b1 = {nextBatchId++, 101, 0, 50, "2025-01-05", 14};
    enqueue(&batchQueues[0], b1);

    Batch b2 = {nextBatchId++, 101, 0, 40, "2025-01-02", 11};
    enqueue(&batchQueues[0], b2);

    Batch b3 = {nextBatchId++, 103, 0, 80, "2024-12-25", 3};
    enqueue(&batchQueues[0], b3);

    Batch b4 = {nextBatchId++, 105, 0, 60, "2025-01-10", 19};
    enqueue(&batchQueues[0], b4);

    // Add sample truck
    strcpy(trucks[0].destination, "Shop #23 - Andheri");
    trucks[0].id = nextTruckId++;
    trucks[0].status = 0; // idle
    initBoxStack(&trucks[0].load);

    strcpy(trucks[1].destination, "Shop #45 - Bandra");
    trucks[1].id = nextTruckId++;
    trucks[1].status = 0;
    initBoxStack(&trucks[1].load);

    truckCount = 2;
}

// ═══════════════════════════════════════════════════════════════════
// DISPLAY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════

void displayDashboard()
{
    Warehouse *w = &warehouses[selectedWarehouse];
    BatchQueue *q = &batchQueues[selectedWarehouse];

    int totalStock = 0;
    int nearExpiry = 0;
    int uniqueProducts = 0;
    int productSeen[MAX_PRODUCTS] = {0};

    // Calculate stats from queue
    for (int i = 0; i < q->count; i++)
    {
        int idx = (q->front + i) % MAX_BATCHES;
        totalStock += q->items[idx].quantity;
        if (q->items[idx].daysUntilExpiry <= 3)
            nearExpiry++;

        int pId = q->items[idx].productId;
        for (int j = 0; j < productCount; j++)
        {
            if (products[j].id == pId && !productSeen[j])
            {
                productSeen[j] = 1;
                uniqueProducts++;
            }
        }
    }

    int trucksOnRoad = 0;
    for (int i = 0; i < truckCount; i++)
    {
        if (trucks[i].status == 2)
            trucksOnRoad++;
    }

    int pendingReturns = 0;
    for (int i = 0; i < returnCount; i++)
    {
        if (returns[i].status == 0)
            pendingReturns++;
    }

    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║                    WAREHOUSE DASHBOARD                    ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");
    printf("    ║  WAREHOUSE: %-20s  TEMP: %3d°C         ║\n", w->name, w->currentTemp);
    printf("    ╠═══════════════════╦═══════════════════╦═══════════════════╣\n");
    printf("    ║  PRODUCTS: %-6d ║  STOCK: %-8d  ║  CAPACITY: %-6d ║\n",
           uniqueProducts, totalStock, w->capacity);
    printf("    ╠═══════════════════╬═══════════════════╬═══════════════════╣\n");
    printf("    ║  NEAR-EXPIRY: %-3d ║  TRUCKS OUT: %-4d ║  RETURNS: %-6d  ║\n",
           nearExpiry, trucksOnRoad, pendingReturns);
    printf("    ╚═══════════════════╩═══════════════════╩═══════════════════╝\n");

    if (nearExpiry > 0)
    {
        printf("\n    ╔═══════════════════════════════════════════════════════════╗\n");
        printf("    ║  ⚠ WARNING: %d batch(es) expiring within 3 days!          ║\n", nearExpiry);
        printf("    ╚═══════════════════════════════════════════════════════════╝\n");
    }
}

void displayQueue(int warehouseIdx)
{
    BatchQueue *q = &batchQueues[warehouseIdx];

    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║              BATCH QUEUE (FIFO - First In First Out)     ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");

    if (isQueueEmpty(q))
    {
        printf("    ║            [ Queue is empty - No batches ]               ║\n");
    }
    else
    {
        printf("    ║  FRONT ──────────────────────────────────────────> REAR  ║\n");
        printf("    ╠═══════════════════════════════════════════════════════════╣\n");

        for (int i = 0; i < q->count; i++)
        {
            int idx = (q->front + i) % MAX_BATCHES;
            Batch *b = &q->items[idx];

            // Find product name
            char productName[MAX_NAME_LEN] = "Unknown";
            for (int j = 0; j < productCount; j++)
            {
                if (products[j].id == b->productId)
                {
                    strcpy(productName, products[j].name);
                    break;
                }
            }

            char marker[10] = "";
            if (i == 0)
                strcpy(marker, "FRONT");
            if (i == q->count - 1)
                strcpy(marker, "REAR");

            if (b->daysUntilExpiry <= 3)
            {
                printf("    ║  [B%03d] %-12s Qty:%-4d Exp:%s ⚠ %s    ║\n",
                       b->id, productName, b->quantity, b->expiryDate, marker);
            }
            else
            {
                printf("    ║  [B%03d] %-12s Qty:%-4d Exp:%s   %s      ║\n",
                       b->id, productName, b->quantity, b->expiryDate, marker);
            }
        }
    }

    printf("    ╚═══════════════════════════════════════════════════════════╝\n");
}

void displayStack(BoxStack *s, const char *title)
{
    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║              %s (LIFO - Last In First Out)       ║\n", title);
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");

    if (isStackEmpty(s))
    {
        printf("    ║                  [ Stack is empty ]                       ║\n");
    }
    else
    {
        printf("    ║                      ┌─────────┐   <- TOP                 ║\n");

        for (int i = s->top; i >= 0; i--)
        {
            Box *box = &s->items[i];

            char productName[MAX_NAME_LEN] = "Unknown";
            for (int j = 0; j < productCount; j++)
            {
                if (products[j].id == box->productId)
                {
                    strcpy(productName, products[j].name);
                    break;
                }
            }

            if (i == s->top)
            {
                printf("    ║                      │ Box %-3d │   %-12s Qty:%-3d ║\n",
                       box->id, productName, box->quantity);
            }
            else
            {
                printf("    ║                      ├─────────┤                       ║\n");
                printf("    ║                      │ Box %-3d │   %-12s Qty:%-3d ║\n",
                       box->id, productName, box->quantity);
            }
        }
        printf("    ║                      └─────────┘   <- BOTTOM              ║\n");
    }

    printf("    ╚═══════════════════════════════════════════════════════════╝\n");
}

void displayProducts()
{
    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║                    PRODUCT CATALOG                        ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");
    printf("    ║  ID   │ NAME           │ TEMP RANGE  │ SHELF LIFE        ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");

    for (int i = 0; i < productCount; i++)
    {
        printf("    ║  %-4d │ %-14s │ %3d to %3d  │ %3d days           ║\n",
               products[i].id, products[i].name,
               products[i].minTemp, products[i].maxTemp,
               products[i].shelfLifeDays);
    }

    printf("    ╚═══════════════════════════════════════════════════════════╝\n");
}

void displayWarehouses()
{
    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║                    WAREHOUSE LIST                         ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");
    printf("    ║  #  │ NAME              │ CURRENT TEMP │ CAPACITY         ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");

    for (int i = 0; i < warehouseCount; i++)
    {
        char selected = (i == selectedWarehouse) ? '*' : ' ';
        printf("    ║ %c%-2d │ %-17s │    %4d°C    │ %4d units        ║\n",
               selected, warehouses[i].id, warehouses[i].name,
               warehouses[i].currentTemp, warehouses[i].capacity);
    }

    printf("    ╚═══════════════════════════════════════════════════════════╝\n");
    printf("    (* = Currently Selected)\n");
}

void displayTrucks()
{
    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║                      TRUCK FLEET                          ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");
    printf("    ║  ID  │ DESTINATION           │ STATUS    │ BOXES LOADED  ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");

    const char *statusNames[] = {"IDLE", "LOADING", "ON-ROAD", "DELIVERED"};

    for (int i = 0; i < truckCount; i++)
    {
        printf("    ║  %-3d │ %-21s │ %-9s │     %-4d        ║\n",
               trucks[i].id, trucks[i].destination,
               statusNames[trucks[i].status], trucks[i].load.top + 1);
    }

    printf("    ╚═══════════════════════════════════════════════════════════╝\n");
}

void displayOperationStack()
{
    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║              OPERATION HISTORY (UNDO STACK)               ║\n");
    printf("    ╠═══════════════════════════════════════════════════════════╣\n");

    if (opStack.top == -1)
    {
        printf("    ║           [ No operations to undo ]                      ║\n");
    }
    else
    {
        for (int i = opStack.top; i >= 0 && i > opStack.top - 5; i--)
        {
            char marker[10] = "";
            if (i == opStack.top)
                strcpy(marker, "<- TOP");
            printf("    ║  %-50s %s  ║\n", opStack.items[i].description, marker);
        }
        if (opStack.top >= 5)
        {
            printf("    ║  ... and %d more operations                             ║\n", opStack.top - 4);
        }
    }

    printf("    ╚═══════════════════════════════════════════════════════════╝\n");
}

// ═══════════════════════════════════════════════════════════════════
// ACTION FUNCTIONS
// ═══════════════════════════════════════════════════════════════════

void addNewBatch()
{
    printHeader("ADD NEW BATCH");
    displayProducts();

    int productId, quantity, daysUntilExpiry;

    printf("\n    Enter Product ID: ");
    scanf("%d", &productId);

    // Verify product exists
    int found = 0;
    for (int i = 0; i < productCount; i++)
    {
        if (products[i].id == productId)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("\n    [!] Product not found!\n");
        pauseScreen();
        return;
    }

    printf("    Enter Quantity: ");
    scanf("%d", &quantity);

    printf("    Days until expiry: ");
    scanf("%d", &daysUntilExpiry);

    Batch newBatch;
    newBatch.id = nextBatchId++;
    newBatch.productId = productId;
    newBatch.warehouseId = selectedWarehouse;
    newBatch.quantity = quantity;
    newBatch.daysUntilExpiry = daysUntilExpiry;
    calculateExpiryDate(newBatch.expiryDate, newBatch.daysUntilExpiry);

    enqueue(&batchQueues[selectedWarehouse], newBatch);

    char opDesc[100];
    sprintf(opDesc, "Added Batch #%d (Qty: %d)", newBatch.id, quantity);
    pushOperation(opDesc, 1, newBatch.id);

    printf("\n");
    printf("    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║  ✓ Batch #%-3d added successfully!                        ║\n", newBatch.id);
    printf("    ║    ENQUEUE operation performed on Batch Queue            ║\n");
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    displayQueue(selectedWarehouse);
    pauseScreen();
}

void removeExpiredBatches()
{
    printHeader("REMOVE EXPIRED BATCHES");

    BatchQueue *q = &batchQueues[selectedWarehouse];
    int removed = 0;

    printf("\n    Scanning for expired batches...\n");
    printf("    ────────────────────────────────\n");

    // Remove batches from front if expired (FIFO - oldest first)
    while (!isQueueEmpty(q) && peekQueue(q).daysUntilExpiry <= 0)
    {
        Batch expired = dequeue(q);
        printf("    [DEQUEUE] Removed expired Batch #%d\n", expired.id);
        removed++;
    }

    if (removed > 0)
    {
        printf("\n   ╔═══════════════════════════════════════════════════════════╗\n");
        printf("    ║  ✓ Removed %d expired batch(es)                            ║\n", removed);
        printf("    ║    DEQUEUE operation performed on Batch Queue             ║\n");
        printf("    ╚═══════════════════════════════════════════════════════════╝\n");

        char opDesc[100];
        sprintf(opDesc, "Removed %d expired batches", removed);
        pushOperation(opDesc, 2, 0);
    }
    else
    {
        printf("\n  ╔═══════════════════════════════════════════════════════════╗\n");
        printf("    ║  ℹ No expired batches found                               ║\n");
        printf("    ╚═══════════════════════════════════════════════════════════╝\n");
    }

    displayQueue(selectedWarehouse);
    pauseScreen();
}

void loadTruck()
{
    printHeader("LOAD TRUCK");
    displayTrucks();

    int truckIdx;
    printf("\n    Select Truck ID: ");
    scanf("%d", &truckIdx);

    // Find truck
    int found = -1;
    for (int i = 0; i < truckCount; i++)
    {
        if (trucks[i].id == truckIdx)
        {
            found = i;
            break;
        }
    }

    if (found == -1)
    {
        printf("\n    [!] Truck not found!\n");
        pauseScreen();
        return;
    }

    trucks[found].status = 1; // loading

    displayQueue(selectedWarehouse);

    BatchQueue *q = &batchQueues[selectedWarehouse];

    if (isQueueEmpty(q))
    {
        printf("\n    [!] No batches available to load!\n");
        pauseScreen();
        return;
    }

    int boxesToLoad;
    printf("\n    How many boxes to load? ");
    scanf("%d", &boxesToLoad);

    printf("\n    Loading truck...\n");
    printf("    ─────────────────\n");

    for (int i = 0; i < boxesToLoad && !isQueueEmpty(q); i++)
    {
        Batch b = dequeue(q);

        Box newBox;
        newBox.id = nextBoxId++;
        newBox.batchId = b.id;
        newBox.productId = b.productId;
        newBox.quantity = b.quantity;

        pushBox(&trucks[found].load, newBox);

        printf("    [DEQUEUE] Batch #%d from queue\n", b.id);
        printf("    [PUSH]    Box #%d onto truck stack\n", newBox.id);
    }

    trucks[found].status = 2; // on-road

    printf("\n    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║  ✓ Truck #%d loaded and dispatched!                        ║\n", trucks[found].id);
    printf("    ║    Queue DEQUEUE + Stack PUSH operations performed        ║\n");
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    displayStack(&trucks[found].load, "TRUCK LOAD");

    char opDesc[100];
    sprintf(opDesc, "Loaded Truck #%d with %d boxes", trucks[found].id, boxesToLoad);
    pushOperation(opDesc, 3, trucks[found].id);

    pauseScreen();
}

void unloadTruck()
{
    printHeader("UNLOAD TRUCK");
    displayTrucks();

    int truckIdx;
    printf("\n    Select Truck ID to unload: ");
    scanf("%d", &truckIdx);

    int found = -1;
    for (int i = 0; i < truckCount; i++)
    {
        if (trucks[i].id == truckIdx)
        {
            found = i;
            break;
        }
    }

    if (found == -1)
    {
        printf("\n    [!] Truck not found!\n");
        pauseScreen();
        return;
    }

    BoxStack *s = &trucks[found].load;

    if (isStackEmpty(s))
    {
        printf("\n    [!] Truck is already empty!\n");
        pauseScreen();
        return;
    }

    displayStack(s, "TRUCK LOAD");

    printf("\n    Unloading boxes (LIFO order)...\n");
    printf("    ────────────────────────────────\n");

    while (!isStackEmpty(s))
    {
        Box box = popBox(s);

        char productName[MAX_NAME_LEN] = "Unknown";
        for (int j = 0; j < productCount; j++)
        {
            if (products[j].id == box.productId)
            {
                strcpy(productName, products[j].name);
                break;
            }
        }

        printf("    [POP] Box #%d: %s (Qty: %d)\n", box.id, productName, box.quantity);
    }

    trucks[found].status = 3; // delivered

    printf("\n    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║  ✓ Truck #%d fully unloaded!                               ║\n", trucks[found].id);
    printf("    ║    Stack POP operations performed (LIFO)                  ║\n");
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    char opDesc[100];
    sprintf(opDesc, "Unloaded Truck #%d at destination", trucks[found].id);
    pushOperation(opDesc, 4, trucks[found].id);

    pauseScreen();
}

void undoLastOperation()
{
    printHeader("UNDO LAST OPERATION");
    displayOperationStack();

    if (opStack.top == -1)
    {
        printf("\n    [!] Nothing to undo!\n");
        pauseScreen();
        return;
    }

    Operation op = popOperation();

    printf("\n    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║  ✓ Undone: %-45s ║\n", op.description);
    printf("    ║    Stack POP operation performed on Undo Stack            ║\n");
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    printf("\n    (In a full implementation, this would reverse the action)\n");

    pauseScreen();
}

void simulateDay()
{
    printHeader("SIMULATE ONE WORKING DAY");

    printf("\n    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║              SIMULATION STARTED...                        ║\n");
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    // Generate random events
    srand(time(NULL));

    printf("\n    ┌───────────────────────────────────────────────────────────┐\n");
    printf("    │ EVENT 1: New Order Received                               │\n");
    printf("    └───────────────────────────────────────────────────────────┘\n");

    int shopNum = rand() % 50 + 1;
    int productIdx = rand() % productCount;
    int orderQty = (rand() % 5 + 1) * 20;

    printf("    From: Shop #%d\n", shopNum);
    printf("    Product: %s (ID: %d)\n", products[productIdx].name, products[productIdx].id);
    printf("    Quantity: %d units\n", orderQty);
    printf("\n    1. Fulfil Order\n");
    printf("    2. Reject Order\n");
    printf("    Your choice: ");

    int choice;
    scanf("%d", &choice);

    if (choice == 1)
    {
        printf("\n    [SIMULATION] Order fulfilled! Dispatching from warehouse...\n");

        char opDesc[100];
        sprintf(opDesc, "Fulfilled order for Shop #%d (%d units)", shopNum, orderQty);
        pushOperation(opDesc, 5, shopNum);
    }
    else
    {
        printf("\n    [SIMULATION] Order rejected.\n");
    }

    printf("\n    ┌───────────────────────────────────────────────────────────┐\n");
    printf("    │ EVENT 2: New Batch Arrival                                │\n");
    printf("    └───────────────────────────────────────────────────────────┘\n");

    int newBatchProduct = rand() % productCount;
    int newBatchQty = (rand() % 10 + 1) * 10;

    printf("    Product: %s\n", products[newBatchProduct].name);
    printf("    Quantity: %d units\n", newBatchQty);
    printf("    Accept delivery? (1=Yes, 2=No): ");

    scanf("%d", &choice);

    if (choice == 1)
    {
        Batch newBatch;
        newBatch.id = nextBatchId++;
        newBatch.productId = products[newBatchProduct].id;
        newBatch.warehouseId = selectedWarehouse;
        newBatch.quantity = newBatchQty;
        newBatch.daysUntilExpiry = rand() % 14 + 7;
        calculateExpiryDate(newBatch.expiryDate, newBatch.daysUntilExpiry);

        enqueue(&batchQueues[selectedWarehouse], newBatch);

        printf("\n    [ENQUEUE] Batch #%d added to queue\n", newBatch.id);

        char opDesc[100];
        sprintf(opDesc, "Received new batch #%d (%d units)", newBatch.id, newBatchQty);
        pushOperation(opDesc, 1, newBatch.id);
    }

    printf("\n    ┌───────────────────────────────────────────────────────────┐\n");
    printf("    │ EVENT 3: Return Request                                   │\n");
    printf("    └───────────────────────────────────────────────────────────┘\n");

    int returnShop = rand() % 50 + 1;
    int returnQty = rand() % 20 + 5;

    printf("    Shop #%d wants to return %d units (damaged)\n", returnShop, returnQty);
    printf("    Accept return? (1=Yes, 2=No): ");

    scanf("%d", &choice);

    if (choice == 1)
    {
        printf("\n    [PENDING] Return added to processing queue\n");
    }

    printf("\n    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║              SIMULATION COMPLETE                          ║\n");
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    displayDashboard();
    pauseScreen();
}
void addWarehouse()
{
    printHeader("ADD NEW WAREHOUSE");

    if (warehouseCount >= MAX_WAREHOUSES)
    {
        printf("\n    [!] Maximum warehouse limit reached!\n");
        pauseScreen();
        return;
    }

    Warehouse w;
    w.id = warehouseCount + 1;

    printf("\n    Enter Warehouse Name: ");
    getchar();
    fgets(w.name, MAX_NAME_LEN, stdin);
    w.name[strcspn(w.name, "\n")] = 0;

    printf("    Enter Current Temperature (°C): ");
    scanf("%d", &w.currentTemp);

    printf("    Enter Capacity: ");
    scanf("%d", &w.capacity);

    warehouses[warehouseCount] = w;
    initQueue(&batchQueues[warehouseCount]);

    warehouseCount++;

    char opDesc[100];
    sprintf(opDesc, "Added Warehouse: %s", w.name);
    pushOperation(opDesc, 6, w.id);

    printf("\n    ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║  ✓ Warehouse '%s' added successfully!                     ║\n", w.name);
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    pauseScreen();
}
void removeWarehouse()
{
    printHeader("REMOVE WAREHOUSE");

    displayWarehouses();

    int choice;
    printf("\n    Enter warehouse number to remove: ");
    scanf("%d", &choice);
    choice--;

    if (choice < 0 || choice >= warehouseCount)
    {
        printf("\n    [!] Invalid warehouse selection!\n");
        pauseScreen();
        return;
    }

    if (batchQueues[choice].count > 0)
    {
        printf("\n    [!] Cannot remove warehouse with inventory!\n");
        pauseScreen();
        return;
    }

    // Shift warehouses left
    for (int i = choice; i < warehouseCount - 1; i++)
    {
        warehouses[i] = warehouses[i + 1];
        batchQueues[i] = batchQueues[i + 1];
    }

    warehouseCount--;

    if (selectedWarehouse >= warehouseCount)
    {
        selectedWarehouse = 0;
    }

    char opDesc[100];
    sprintf(opDesc, "Removed Warehouse #%d", choice + 1);
    pushOperation(opDesc, 7, choice + 1);

    printf("\n  ╔═══════════════════════════════════════════════════════════╗\n");
    printf("    ║  ✓ Warehouse removed successfully!                        ║\n");
    printf("    ╚═══════════════════════════════════════════════════════════╝\n");

    pauseScreen();
}

// ═══════════════════════════════════════════════════════════════════
// ROLE MENUS
// ═══════════════════════════════════════════════════════════════════

void adminMenu()
{
    int choice;

    do
    {
        printHeader("ADMIN MENU");
        displayDashboard();

        printf("\n");
        printf("    ╔═══════════════════════════════════════╗\n");
        printf("    ║  [1] View All Warehouses              ║\n");
        printf("    ║  [2] View All Products                ║\n");
        printf("    ║  [3] View System Logs                 ║\n");
        printf("    ║  [4] Undo Last Operation              ║\n");
        printf("    ║  [5] Select Warehouse                 ║\n");
        printf("    ║  [6] Add Warehouse                    ║\n");
        printf("    ║  [7] Remove Warehouse                 ║\n");
        printf("    ║  [8] Back to Role Selection ➤         ║\n");
        printf("    ╚═══════════════════════════════════════╝\n");
        printf("\n    Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printHeader("ALL WAREHOUSES");
            displayWarehouses();
            pauseScreen();
            break;
        case 2:
            printHeader("ALL PRODUCTS");
            displayProducts();
            pauseScreen();
            break;
        case 3:
            printHeader("SYSTEM LOGS");
            displayOperationStack();
            pauseScreen();
            break;
        case 4:
            undoLastOperation();
            break;
        case 5:
            printHeader("SELECT WAREHOUSE");
            displayWarehouses();
            printf("\n    Enter warehouse number (1-%d): ", warehouseCount);
            scanf("%d", &selectedWarehouse);
            selectedWarehouse--;
            if (selectedWarehouse < 0 || selectedWarehouse >= warehouseCount)
            {
                selectedWarehouse = 0;
            }
            break;
        case 6:
            addWarehouse();
            break;
        case 7:
            removeWarehouse();
            break;
        case 8:
            break;
        default:
            printf("\n    [!] Invalid choice!\n");
            pauseScreen();
        }
    } while (choice != 8);
}

void warehouseManagerMenu()
{
    int choice;

    do
    {
        printHeader("WAREHOUSE MANAGER MENU");
        displayDashboard();

        printf("\n");
        printf("    ╔═══════════════════════════════════════╗\n");
        printf("    ║  [1] View Inventory (Batch Queue)     ║\n");
        printf("    ║  [2] Add New Batch (ENQUEUE)          ║\n");
        printf("    ║  [3] Remove Expired (DEQUEUE)         ║\n");
        printf("    ║  [4] Load Truck (PUSH to Stack)       ║\n");
        printf("    ║  [5] View All Trucks                  ║\n");
        printf("    ║  [6] Simulate One Day                 ║\n");
        printf("    ║  [7] Back to Role Selection ➤         ║\n");
        printf("    ╚═══════════════════════════════════════╝\n");
        printf("\n    Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            printHeader("BATCH INVENTORY");
            displayQueue(selectedWarehouse);
            pauseScreen();
            break;
        case 2:
            addNewBatch();
            break;
        case 3:
            removeExpiredBatches();
            break;
        case 4:
            loadTruck();
            break;
        case 5:
            printHeader("TRUCK FLEET");
            displayTrucks();
            pauseScreen();
            break;
        case 6:
            simulateDay();
            break;
        case 7:
            break;
        default:
            printf("\n    [!] Invalid choice!\n");
            pauseScreen();
        }
    } while (choice != 7);
}

void truckDriverMenu()
{
    int choice;

    do
    {
        printHeader("TRUCK DRIVER MENU");
        displayTrucks();

        printf("\n");
        printf("    ╔═══════════════════════════════════════╗\n");
        printf("    ║  [1] View Truck Load (Stack)          ║\n");
        printf("    ║  [2] Unload at Destination (POP)      ║\n");
        printf("    ║  [3] Back to Role Selection  ➤        ║\n");
        printf("    ╚═══════════════════════════════════════╝\n");
        printf("\n    Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
        {
            int truckIdx;
            printf("\n    Enter Truck ID: ");
            scanf("%d", &truckIdx);

            int found = -1;
            for (int i = 0; i < truckCount; i++)
            {
                if (trucks[i].id == truckIdx)
                {
                    found = i;
                    break;
                }
            }

            if (found != -1)
            {
                printHeader("TRUCK LOAD VIEW");
                displayStack(&trucks[found].load, "TRUCK LOAD");
            }
            else
            {
                printf("\n    [!] Truck not found!\n");
            }
            pauseScreen();
            break;
        }
        case 2:
            unloadTruck();
            break;
        case 3:
            break;
        default:
            printf("\n    [!] Invalid choice!\n");
            pauseScreen();
        }
    } while (choice != 3);
}

// ═══════════════════════════════════════════════════════════════════
// MAIN FUNCTION
// ═══════════════════════════════════════════════════════════════════

int main()
{
    initializeData();

    int roleChoice;

    do
    {
        clearScreen();
        printMinecraftBanner();

        printf("    ╔═══════════════════════════════════════════════════════════╗\n");
        printf("    ║                  SELECT YOUR ROLE  ➤                      ║\n");
        printf("    ╠═══════════════════════════════════════════════════════════╣\n");
        printf("    ║                                                           ║\n");
        printf("    ║     [1] 👤 Admin                                          ║\n");
        printf("    ║         - Manage warehouses, products, system logs        ║\n");
        printf("    ║                                                           ║\n");
        printf("    ║     [2] 🏭 Warehouse Manager                              ║\n");
        printf("    ║         - Inventory, batches, dispatch orders 📦          ║\n");
        printf("    ║                                                           ║\n");
        printf("    ║     [3] 🚚 Truck Driver                                   ║\n");
        printf("    ║         - Load/unload trucks, deliveries                  ║\n");
        printf("    ║                                                           ║\n");
        printf("    ║     [4] ↩️  Exit                                           ║\n");
        printf("    ║                                                           ║\n");
        printf("    ╚═══════════════════════════════════════════════════════════╝\n");
        printf("\n    Enter choice: ");
        scanf("%d", &roleChoice);

        switch (roleChoice)
        {
        case 1:
            adminMenu();
            break;
        case 2:
            warehouseManagerMenu();
            break;
        case 3:
            truckDriverMenu();
            break;
        case 4:
            clearScreen();
            printf("\n");
            printf("    ╔═══════════════════════════════════════════════════════════╗\n");
            printf("    ║                                                           ║\n");
            printf("    ║        Thank you for using Cold-Chain Simulator! :D       ║\n");
            printf("    ║                                                           ║\n");
            printf("    ║              Data Structures Demonstrated:                ║\n");
            printf("    ║              - QUEUE (FIFO) for Batch Management          ║\n");
            printf("    ║              - STACK (LIFO) for Truck Loading             ║\n");
            printf("    ║              - STACK for Undo Operations                  ║\n");
            printf("    ║                                                           ║\n");
            printf("    ╚═══════════════════════════════════════════════════════════╝\n");
            printf("\n");
            break;
        default:
            printf("\n    [!] Invalid choice. Try again.\n");
            pauseScreen();
        }
    } while (roleChoice != 4);

    return 0;
}
