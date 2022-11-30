#include "bptree.h"
#include <vector>
#include <sys/time.h>

struct timeval
cur_time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}

void
print_tree_core(NODE *n)
{
	printf("["); 
	for (int i = 0; i < n->nkey; i++) {
		if (!n->isLeaf) print_tree_core(n->chi[i]); 
		printf("%d", n->key[i]); 
		if (i != n->nkey-1 && n->isLeaf) putchar(' ');
	}
	if (!n->isLeaf) print_tree_core(n->chi[n->nkey]);
	printf("]");
}

void
print_tree(NODE *node)
{
	print_tree_core(node);
	printf("\n"); fflush(stdout);
}

NODE *
find_leaf(NODE *node, int key)
{
	int kid;

	if (node->isLeaf) return node;
	for (kid = 0; kid < node->nkey; kid++) {
		if (key < node->key[kid]) break;
	}

	return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
	int i;
	int j;
	if (key < leaf->key[0]) {
		for (i = leaf->nkey; i > 0; i--) {
			leaf->chi[i] = leaf->chi[i-1] ;
			leaf->key[i] = leaf->key[i-1] ;
		} 
		leaf->key[0] = key;
		leaf->chi[0] = (NODE *)data;
	}
	else {
		for (i = 0; i < leaf->nkey; i++) {
			if (key < leaf->key[i]) break;
		}
		for (j = leaf->nkey; j > i; j--) {		
			leaf->chi[j] = leaf->chi[j-1] ;
			leaf->key[j] = leaf->key[j-1] ;
		} 
		leaf->key[j] = key;
		leaf->chi[j] = (NODE * ) data;
	}
	leaf->nkey++;

	return leaf;
}


TEMP * alloc_temp(NODE *memory)
{
	// store the data being split 
	TEMP *node;
	if (!(node = (TEMP*)calloc(1, sizeof(TEMP)))) ERR;
	node->isLeaf = true;
	node->nkey = 0;
	
	return node;
}

NODE *
alloc_leaf(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
	node->isLeaf = true;
	node->parent = parent;
	node->nkey = 0;

	return node;
}

void insert_temp(TEMP *M, NODE *node)
{
	for (int i = 0; i < node->nkey; i++){
		M->chi[i] = node->chi[i];
		M->key[i] = node->key[i];
	}
	// check
	M->nkey = node->nkey; 
}

TEMP * place_temp(TEMP *M, int key,DATA *data)
{
	// essentially same as inserting into leaf (allowing full leaves too)
	int i, j;
	if (key < M->key[0])
	{
		for (i = M->nkey; i > 0; i--)
		{
			M->chi[i] = M->chi[i-1];
			M->key[i] = M->key[i-1];
		}
		M->key[0] = key;
		M->chi[0] = (NODE *) data;
	}
	else 
	{
		for (i = 0; i < M->nkey; i++)
		{
			if (key < M->key[i]) break;
		}
		for (j = M->nkey; j>=i; j--)
		{
			M->chi[j] = M->chi[j-1];
			M->key[j] = M->key[j-1];
		}
		M->key[i] = key;
		M->chi[i] = (NODE *) data;
	}
	return M;
}

void delete_leaf(NODE *node)
{
	int i;
	for (i = 0; i < N - 2; i++) {
		node->chi[i] = nullptr;
	}
	for (i = 0; i < N - 1; i++) {
		node->key[i] = 0; 
	}
	node->nkey = 0;
}

void ptr_new(NODE *leaf, NODE *new_leaf)
{
	new_leaf->chi[N-1] = leaf->chi[N-1];
	leaf->chi[N-1] = new_leaf;
	new_leaf->parent = leaf;
}

void parent_insert(NODE *leaf, NODE *new_leaf, int key)
{
	int i, j;
	int k = ceil(( N + 1 ) / 2);
	
	if (leaf == Root) 
	{
		// setup new leaf node
		NODE *RT = alloc_leaf(NULL);
		RT->key[0] = key;
		RT->chi[0] = leaf;
		RT->chi[1] = new_leaf;
		leaf->parent = RT;
		new_leaf->parent = RT;
		RT->nkey++;

		Root = RT;
		RT->isLeaf = false;
		return;
	}


	// 
	leaf->parent = Root;
	if (Root-> nkey < N - 1)
	{
		if (new_leaf->key[0] < Root->key[0])
		{
			for (i = Root->nkey; i > 0; i--)
			{
				Root->key[i] = Root->key[i - 1];
				Root->chi[i + 1] = Root->chi[i];
			}
			Root->key[0] = new_leaf->key[0];
			Root->chi[1] = new_leaf;
		}
		else
		{
			for (i = 0; i <= Root->nkey; i++)
			{
				if (new_leaf->key[0] < Root->key[i]) break;
			}
			for (j = Root->nkey; j>= i; j--)
			{
				Root->chi[j + 1] = Root->chi[j] ;
				Root->key[j] = Root->key[j-1];
			}
			Root->key[j] = new_leaf->key[0];
			Root->chi[j + 1] = new_leaf;
		}


		Root->nkey++;
		return;
	}
	else
	{
		TEMP *temp = alloc_temp(NULL);
		insert_temp(temp, Root);

		if (new_leaf->key[0] < temp->key[0])
		{
			for (i = temp->nkey; i > 0; i--)
			{
				temp->key[i] = temp->key[i-1];
				temp->chi[i+1] = temp->chi[i];
			}
			temp->key[0] = new_leaf->key[0];
			temp->chi[1] = new_leaf;	
		}
		else
		{
			for (i = 0; i <= temp->nkey; i++){
				if (new_leaf->key[0] < temp-> key[i]) break;
			}
			for (j = temp->nkey; j >= i; j--) 
			{
				temp->chi[j + 1] = temp->chi[j];
				temp->key[j] = temp->key[j-1];
			}
			temp->key[j] = new_leaf->key[0];
			temp->chi[j] = new_leaf;
		}
		temp->nkey++;

		delete_leaf(Root);
		NODE *parent = alloc_leaf(NULL);
		parent->isLeaf = false;
		
		for (i = 0; i < k ; i++)
		{
			Root->chi[i] = temp->chi[i];
			Root->key[i] = temp->key[i];
			Root->nkey++;
		}
		int A = temp->key[k];
		int l = 0;

		for (j = k + 1; j <= N; j++) {
			parent->chi[l] = temp->chi[j];
			l++;
		}
		parent->key[0] = temp->key[k +1];
		parent->nkey++;
	
	}
}

/*----------------------*//*----------------------*//*----------------------*//*----------------------*//*----------------------*/

void 
insert(int key, DATA *data)
{
	NODE *leaf;

	if (Root == NULL) {
		leaf = alloc_leaf(NULL);
		Root = leaf;
	}
  else {
    leaf = find_leaf(Root, key);
  }
	if (leaf->nkey < (N-1)) { // if there is space for the leaf, than normal insert can occur
		insert_in_leaf(leaf, key, data);
	}
	else { 
		// split has to occur
		int i, j, new_key;
		int k = 0;
		// create a new leaf and temporary M memory for the N data from the insertion.
		NODE *new_leaf = alloc_leaf(NULL);
		TEMP *temp_leaf = alloc_temp(NULL);

		// insert the data into the temp while storing the new key and data into the temp leaf
		insert_temp(temp_leaf, leaf);
		place_temp(temp_leaf, key, data);

		//set the ptr to  
		ptr_new(leaf, new_leaf);
		delete_leaf(leaf);


		//left side
		for (i = 0; i < ceil(N/2); i++)
		{
			leaf->chi[i] = temp_leaf->chi[i];
			leaf->key[i] = temp_leaf->key[i];
			leaf->nkey++;
		}

		//right side
		for (j = ceil(N/2); j < N; j++)
		{
			new_leaf->chi[k] = temp_leaf->chi[j];
			new_leaf->key[k] = temp_leaf->key[j];
			new_leaf->nkey++;

			k++;
		}

		// set the new_key to be the min for the new_leaf
		new_key = new_leaf->key[0];
		parent_insert(leaf, new_leaf, new_key);
	}
}

void
init_root(void)
{
	Root = NULL;
}

int 
interactive()
{
  int key;

  std::cout << "Key: ";
  std::cin >> key;

  return key;
}

int
main(int argc, char *argv[])
{
  struct timeval begin, end;

	init_root();

	printf("-----Insert-----\n");
	begin = cur_time();
  while (true) {
		insert(interactive(), NULL);
    print_tree(Root);
  }
	end = cur_time();

	return 0;
}
