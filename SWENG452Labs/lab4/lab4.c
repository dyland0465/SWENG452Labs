#include <stdio.h>
#include <stdlib.h>

struct ListNode {
  char data;
  struct ListNode *nextPtr;
};

typedef struct ListNode ListNode;
typedef ListNode *ListNodePtr;

void printList(struct ListNode *first) {
  struct ListNode *current = first;
  while (current != NULL) {
    printf("%c ", current->data);
    current = current->nextPtr;
  }
  printf("*\n");
}

struct ListNode *deleteNode(struct ListNode *first, int pos) {

  if (first == NULL)
    return first;

  struct ListNode *temp = first;

  if (pos == 0) {
    first = temp->nextPtr;
    free(temp);
    return first;
  }

  struct ListNode *prev = NULL;

  for (int i = 0; temp != NULL && i < pos; i++) {
    prev = temp;
    temp = temp->nextPtr;
  }

  if (temp == NULL) {
    printf("Out of range\n");
    return first;
  }
  prev->nextPtr = temp->nextPtr;
  free(temp);
  return first;
}

struct ListNode *insertNode(struct ListNode *first, int index, char val) {
  struct ListNode *newNode = malloc(sizeof(struct ListNode));
  newNode->data = val;

  if (index == 0) {
    newNode->nextPtr = first;
    return newNode;
  }

  struct ListNode *current = first;

  for (int i = 0; current != NULL && i < index - 1; i++) {
    current = current->nextPtr;
  }

  if (current == NULL) {
    printf("Invalid insert\n");
    free(newNode);
    return first;
  }

  newNode->nextPtr = current->nextPtr;
  current->nextPtr = newNode;
  return first;
}

int main() {
  struct ListNode *node = NULL;
  struct ListNode *prev = NULL;
  int initNodes;
  int index;
  char val;
  int input;

  printf("Please enter the number of initial nodes: ");
  scanf("%d", &initNodes);

  for (int i = 0; i < initNodes; i++) {
    struct ListNode *newNode = malloc(sizeof(struct ListNode));

    printf("Enter data for node %d: ", i);
    scanf(" %c", &val);
    newNode->data = val;
    newNode->nextPtr = NULL;
    if (node == NULL) {
      node = newNode;
      prev = node;
    } else {
      prev->nextPtr = newNode;
      prev = newNode;
    }
  }
  while (1) {
    printf("\nLinked List Menu: \n");
    printf("1: Insert Node. \n");
    printf("2: Delete Node. \n");
    printf("3. Print List. \n");
    printf("4. Exit Program. \n");
    printf("User choice: ");
    // scanf("%d", input);

    if (scanf("%d", &input) != 1)
      break;

    if (input == 4) {
      printf("\nThank you! \n");
      break;
    }
    switch (input) {
    case 1: // Insert
      printf("Index to insert ");
      scanf("%d", &index);
      printf("data to insert: ");
      scanf(" %c", &val);
      node = insertNode(node, index, val);
      break;
    case 2: // delete
      printf("Index to delete ");
      scanf("%d", &index);
      printf("deleting %d from list...\n", index);
      node = deleteNode(node, index);
      break;
    case 3: // print
      printList(node);
      break;
    default:
      printf("Invalid option. Please try again.\n");
      break;
    }
  }
}
