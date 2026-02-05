#include <stdio.h>
#include <stdlib.h>

struct ListNode {
  char data;
  struct ListNode *nextPtr;
};

typedef struct ListNode ListNode;
typedef ListNode *ListNodePtr;

struct ListNode *deleteNode(struct ListNode *first, int pos) {
  struct ListNode *temp = first;

  if (pos == 1) {
    first = temp->nextPtr;
    free(temp);
    return first;
  }

  struct ListNode *prev = NULL;

  for (int i = 1; temp != NULL && i < pos; i++) {
    prev = temp;
    temp = temp->nextPtr;
  }

  if (temp == NULL) {
    printf("Out of range");
    return first;
  }
  prev->nextPtr = temp->nextPtr;
  free(temp);
  return first;
}

void printList(struct ListNode *first) {
  while (first != NULL) {
    printf("%s", first->data);
    printf(" ");
  }
  printf("*\n");
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

    printf("Enter data for node %d: ", i + 1);
    scanf("%c", &val);
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
    scanf("%d", input);

    if (input == 4)
      break;

    switch (input) {
    case 1: // Insert
      printf("Index to insert");
      scanf("%d", index);
      printf("data to insert: ");
      scanf("%s", val);
      node->data = val;
      node = node->nextPtr;
      break;
    case 2: // delete
      printf("Index to delete");
      scanf("%d", &index);
      printf("deleting %s from list\n", val);
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
