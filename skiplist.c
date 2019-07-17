
/*
* File: 	SkipList.c
* Author: 	Fanny Avila & Marcos Avila
* Description:	This program implements a Skip List. A Skip List is an advanced
*		data structure that allows for fast search on an ordered list 
*		of elements. This is possible because a skip list maintains a 
*		heiarchy of sublists that each successivly skip over fewer and 
*		fewer elements as well as the full list of elements. This 
*		feature of the skip list allows us to skip over sections of 
*		elements when searching for elements. When searching for an 
*		element in a skip list, the node with the least nodes will be 
*		searched first, it will compare the elements in the sub list 
*		until 2 consecutiv elements have been found, one that is 
*		smaller and one that is larger or equal to the element being 
*		searched for. the search will then continue to be refined 
*		through searching any other sublist, each time skiping groups 
*		of elements that are smaller than the search element until
*		the element is found or found to be not in the list.
*		
*		This skip list chooses the element that are skipped over 
*		probabilistically as opposed to deterministically. we use a
*		simulated "coinflip" to determine the number of lists an added 
*		element will appear in. 
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* _sl_node Skip 
* List node. A skip list node needs to be accessable from 4 directions. we need to
* be able to go "forward" and "backward" within a sub list as well as "up" and 
* "down" to travel between sublists. Each node also needs to hold data.
*/

struct _sl_node {
	struct _sl_node *_prev_node;
	struct _sl_node *_next_node;
	struct _sl_node *_prev_layer;
	struct _sl_node *_next_layer;
	void *_data;
};

/* skip_list 
* A Skip list needs a pointer to the head list, access to the comparison
* function, and a size attribute that needs to be maintained. 
*
*/

struct skip_list {
	struct _sl_node *_first_node;
	int (*_gt_func)(void *, void *);
	int _size;
};

/*private functions*/

/* _coin_flip 
* The coin flip algorithm returns either a 1 or a 0. This is used to
* determine if an added element will appear in subsequent sublists. If the coin
* flip returns a 1 "heads" the element will be added to the next sublist. If
* the coin flip returns a 0 "tails", the element will not be added to any other
8* sublists. 
*/

int _coin_flip() {
	return rand() % 2;
}

/* 
* This private function returns a pointer to the node previous the node 
* containing data that is gt or equal to the data we are searching for. This is
* useful in the case of inserting and deleting a node. This is implemented 
* using a recursive function.  
* 
* Arguments: 
*	int (*gt_func)(void*, void*) - pointer to the gt_func which takes in 2 
*		void pointers and returns a 1 if the first arg is greater than 
*		the second arg or a 0 otherwise.
*	struct _sl_node *current_node - pointer to current_node that needs to 	
*		be checked. 
*	void *data - pointer to the data we are searching for. 
* Returns:
*	struct _sl_node * - pointer to node location before the location of node
*		with data that is gt or equal to the comparison data
*/

struct _sl_node *_find_previous( int (*gt_func)(void *, void *), 
		struct _sl_node *current_node, 
		void *data
) {
	struct _sl_node *temp_node;
	temp_node = current_node;
	
	//searches sublist
	while(temp_node->_next_node && gt_func(data, temp_node->_next_node->_data)) {
		temp_node = temp_node->_next_node;
	}

	// if l0 has not been reached, go to next sublist
	if(!(temp_node->_next_layer)) {	
		return temp_node;
	}

	return _find_previous(gt_func, temp_node->_next_layer, data);   
}     
/*
* This private function uses recursion to find the first instance of node 
* containing the specified data
*
*/
struct _sl_node *_find_node( int (*gt_func)(void *, void *),
		struct _sl_node *current_node, 
		void *data
) {
	struct _sl_node *temp_node;
	temp_node = current_node;
	
	while(temp_node->_next_node && gt_func(data, temp_node->_next_node->_data)) {
		temp_node = temp_node->_next_node;
		return temp_node;
	}
	
	if(!(temp_node->_next_layer)) {	
		return temp_node;
	}

	return _find_previous(gt_func, temp_node->_next_layer, data);   
}

/*
* This private function deletes a specific node from the list, including all 
* the sublists using a recursive function. The algorithm starts from the base 
* list "l0", that contains all the nodes to go up. Because the pointer to the 
* exact node to be deleted is provided, we rearange the pointers of any nodes 
* connected to the deleted node and free the deleted node. 
* 
* Arguments: 
*	struct _sl_node *current_node - pointer to the node containing the data
*		that needs to be deleted
*/

void _delete_node(struct _sl_node *del_sl_node) {
	struct _sl_node *temp_prev_node;

	if(!del_sl_node) {
		return;
	}

	_delete_node(del_sl_node->_prev_layer);	//recursive call

	temp_prev_node = del_sl_node->_prev_node; // set a temp pointer to previous node 
	temp_prev_node->_next_node = del_sl_node->_next_node; // set _next_node of temp node to the delete node's _next_node. Will set the next pointer to null if delete_node has no _next_node
	
	if(del_sl_node->_next_node) { // if there is a next node, it's previous pointer needs to be maintained.
		del_sl_node->_next_node->_prev_node = temp_prev_node;
	}
  
	free(del_sl_node); //deallocate memory
}

/* 
* This private function inserts a new node into the list, including all 
* the sublists using a recursive function. Using the previous node pointer we 
* can allocate space for the new node and rearange pointers to connect to the 
* new node. We then use the _coin_flip() to determine how tall the new node's 
* column will be.(how many sublists contain the new node and if we need to 
* create new sublists). This is a recursive function. 
*
* Arguments: 
*	struct _sl_node *prev_node - pointer to the node containing the data
*		that needs to be inserted
*	struct _sl_node *next_layer - pointer to the next sub list.
*	void *data - poiner to data that needs to be in the new node.
*
* Return:
*	struct _sl_node * - returns a pointer to the newset node to be used 
*		when determining height of the new node's colomn.	
*/

struct _sl_node *_insert_node(struct _sl_node *prev_node, struct _sl_node *next_layer, void *data) {
	struct _sl_node *new_node; 
	struct _sl_node *new_layer;
	struct _sl_node *temp_node;
	
	//initialize variables
	new_node = (struct _sl_node *)malloc(sizeof(struct _sl_node));
	new_node->_prev_node = prev_node;
	new_node->_next_node = prev_node->_next_node;
	new_node->_prev_layer = NULL;
	new_node->_next_layer = next_layer;
	new_node->_data = data;

	prev_node->_next_node = new_node;
    	
	// if the the new_node has a next node, it's previous pointer needs to be updated
	if(new_node->_next_node) {
		new_node->_next_node->_prev_node = new_node;
    	}
	
	//check if node should be added to next sublist 
	if(_coin_flip()) {
		temp_node = prev_node; 
		
		while(!(temp_node->_prev_layer)) {
			if(!(temp_node->_prev_node)) {
				
				// initilizing new node
				new_layer = (struct _sl_node *)malloc(sizeof(struct _sl_node));
				new_layer->_prev_node = NULL;
				new_layer->_next_node = temp_node->_next_node;
				new_layer->_prev_layer = temp_node;
				new_layer->_next_layer = temp_node->_next_layer;
				new_layer->_data = NULL;
				 
				if(temp_node->_next_node) {
					temp_node->_next_node->_prev_node = new_layer;
				}
				
				if(temp_node->_next_layer) {
					temp_node->_next_layer->_prev_layer = new_layer;
				}

				temp_node->_next_node = NULL;
				temp_node->_next_layer = new_layer;
				temp_node = new_layer;
				break;
			}
			
			temp_node = temp_node->_prev_node;
		}
		
		temp_node = temp_node->_prev_layer;
		new_node->_prev_layer = _insert_node(temp_node, new_node, data);
	}
	
	return new_node;
}

/* 
* This private function reduces the height of the sublists after deleting a node;
* Arguments: 
*	_sl_node * head_node - the head node of a sublist which is always NULL
* Return:
*	_sl_node * - returns head node of a sub list that should be deleted.
*/
struct _sl_node *_reduce_height(struct _sl_node *head_node) {
	struct _sl_node *temp_next_layer;

	if(!(head_node->_next_layer) || (head_node->_next_node)) {
		return head_node;
	}

	temp_next_layer = head_node->_next_layer;
	free(head_node);  	
	temp_next_layer->_prev_layer = NULL;
	
	return _reduce_height(temp_next_layer);
}
/*
* This private function deallocates the memeory used for the skip list using a
* recursive function. 
* 
* Arguments:
* 	struct _sl_node *current_node - first node of the skiplist that needs
*		to be deleted.
*/
void _delete_skip_list(struct _sl_node *current_node) {
	if(!current_node) {
		return;
	}

	_delete_skip_list(current_node->_next_layer);
	_delete_skip_list(current_node->_next_node);

	if(current_node->_prev_layer){
		current_node->_prev_layer->_next_layer = NULL;
	}
	free(current_node);
}

/*public functions - construction and destruction functions*/

//constructor

/*
* public function that initializes a new skip list
* 
* Arguments:
* 	int (*gt_func)(void *, void *) - pointer to the greater than function
*		to be used when initilizing a skip list
* Return:
	struct skip_list * - pointer to a new skip list 
*/

struct skip_list *skip_list_create(int (*gt_func)(void *, void *)) {
	// initialize skip list structure
	srand(time(NULL));
	struct skip_list *new_skip_list = (struct skip_list *)malloc(sizeof(struct skip_list));
	new_skip_list->_gt_func = gt_func;
	new_skip_list->_size = 0;

	// initialize first node [header doubly linked-list]
	new_skip_list->_first_node = (struct _sl_node *)malloc(sizeof(struct _sl_node));
	new_skip_list->_first_node->_prev_node = NULL;
	new_skip_list->_first_node->_next_node = NULL;
	new_skip_list->_first_node->_prev_layer = NULL;
	new_skip_list->_first_node->_next_layer = NULL;
	new_skip_list->_first_node->_data = NULL;

	return new_skip_list;
}

// Destructor

/*
* public function that dealocates memory used in a skip_list
* 
* Arguments:
* 	struct skip_list *del_skip_list - pointer skip_list to be deleted
* Return:
	int - returns 0 if function was executed succesfully
*/

int skip_list_destroy(struct skip_list *del_skip_list) {
	_delete_skip_list(del_skip_list->_first_node);	// destroy skip list
	free(del_skip_list);	// destroy container structure
	return 0;
}

/*public functions - access functions*/

/* 
* public functiion that returns size of skip list
*
* Arguments:
*	struct skip_list *sl - pointer to skip list
* Returns:
*	int - returns the size of the skip list
*/

int skip_list_size(struct skip_list *sl) {
	return sl->_size;
}


/* 
* public functiion that searches a list for specified data
*
* Arguments:
*	struct skip_list *sl - pointer to skip list
*	void *data - pointer to specified data the function needs to search for
* Returns:
*	int - returns 0 if data is not in the list 1 if it is.
*/

int skip_list_contains(struct skip_list *sl, void *data) {
	struct _sl_node *prev_node;

	// Find node before where "data" should be
	prev_node = _find_previous(sl->_gt_func, sl->_first_node, data);

	// Next node is NULL
	if(!(prev_node->_next_node)) {
		return 0;
	}

	// Next node contains "data"
	return (prev_node->_next_node->_data) == data;
}

/*public functions - modification functions*/

/* 
* public functiion that removes specified data from the skip list.
*
* Arguments:
*	struct skip_list *sl - pointer to skip list
	void *data - pointer to specifed data the function needs to search for
* Returns:
*	int - returns 0 if data did not exist or not removed, 1 if data was 
*		removed
*/

int skip_list_remove(struct skip_list *sl, void *data) {
	struct _sl_node *prev_node;

	// Find node before where "data" should be
	prev_node = _find_previous(sl->_gt_func, sl->_first_node, data);

	// Next node is NULL or next node is not "data"
	if(!(prev_node->_next_node) || (prev_node->_next_node->_data) != data) {
		return 0;
	}

	// Remove node entirely from skip list
	_delete_node(prev_node->_next_node);

	// Reduce skip list height
	sl->_first_node = _reduce_height(sl->_first_node);
	--(sl->_size);

	return 1;
}

/* 
* public functiion that inserts specified data from the skip list.
*
* Arguments:
*	struct skip_list *sl - pointer to skip list
*	void *data - pointer to specifed data the function needs to be added to
*		the list.
* Returns:
*	int - returns 0 if data already existed in the list, 1 if data was 
*		succesfully added.
*/

int skip_list_insert(struct skip_list *sl, void *data) {
	struct _sl_node *prev_node;

	// Find node before where data should be
	prev_node = _find_previous(sl->_gt_func, sl->_first_node, data);

	// Next node is not NULL and data is already inside of skip list
	if(prev_node->_next_node && (prev_node->_next_node->_data) == data) {
		return 0;
	}
    
	_insert_node(prev_node, NULL, data);
	++(sl->_size);

	return 1;
}

/* 
* public functiion that prints out the list in rows and columns to improve 
* readability when testing. The colums let you see the sublists more clearly. 
*
* Arguments:
*	struct skip_list *sl - pointer to skip list
*/

void skip_list_print(struct skip_list *sl) {
	//calculate pointer to L0 list
	struct _sl_node *l0_list = sl->_first_node;
	struct _sl_node *current_layer = sl->_first_node;
	struct _sl_node *current_node = sl->_first_node;
	struct _sl_node *l0_list_node = NULL;	

	while(l0_list->_next_layer){
		l0_list=l0_list->_next_layer;
	}

	l0_list_node=l0_list;
	if(current_node->_next_node){// if current list is not empty	
		while(current_node || current_layer->_next_layer){
			if(current_node){
				while(sl->_gt_func(current_node->_data, l0_list_node->_next_node->_data) && l0_list_node->_next_node){
					printf("-\t-");
					l0_list_node=l0_list_node->_next_node;				
				}
				printf("->%ld", (long)current_node->_data);
				current_node=current_node->_next_node;
			}
			if(!(current_node)&&(current_layer->_next_layer)){
				current_layer=current_layer->_next_layer;
				current_node=current_layer;
				l0_list_node=l0_list;
				printf("\n");		
			}
		}
		printf("\n");			
		return;	
	}
}
int fifo_gt(void *a, void *b) {return (long)a > (long)b;}

int main() {
	struct skip_list *test_list;
	test_list = skip_list_create(fifo_gt);

	for(long i = 0; i < 30; i += 2)
	{
		skip_list_insert(test_list, (void *)i);
	}

	for(long j = 0; j < 30; ++j)
	{
		printf("Contains %ld: %d\n", j, skip_list_contains(test_list, (void *)j));
	}

	skip_list_print(test_list);


	for(long i = 0; i < 30; i += 4)
	{
		skip_list_remove(test_list, (void *)i);
	}

	printf("\n");
	skip_list_print(test_list);

	skip_list_destroy(test_list);

	return 0;
}
