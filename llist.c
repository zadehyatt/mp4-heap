/* llist.c                  
 * Zade Hyatt
 * zadeh                
 * zadehyatt
 * ECE 2230 Fall 2025
 * Lab4: Dynamic Memory Allocation
 *
 * Purpose: To provie a reusable, generic linked list data structure that can store
 * any type of data and supports sorted and unsorted lists. 
 *
 * Assumptions: 
 *  - The list can store any type of data
 *  - The list can be sorted or unsorted
 *  - The list can be accessed at the front, back, or at specific index
 *  
 * 
 *
 * Bugs: 
 *
 */
#define _GNU_SOURCE   // so that stdlib.h defines qsort_r
#include <stdlib.h>
#include <assert.h>

#include "llist.h"        // defines public functions for list ADT
#include <stdio.h>
// definitions for private constants used in llist.c only

#define LLIST_SORTED    989898
#define LLIST_UNSORTED -898989

// prototypes for private functions used in llist.c only

void llist_debug_validate(llist_t *L);

// MP3 sorting algorithm prototypes
void llist_insertion_sort(llist_t *list_ptr);
void llist_recursive_selection_sort(llist_t *list_ptr);
void llist_recursive_selection_sort_helper(llist_t *list_ptr, llist_elem_t *m, llist_elem_t *n);
void llist_iterative_selection_sort(llist_t *list_ptr);
void llist_merge_sort(llist_t *list_ptr);
void llist_quick_sort(llist_t *list_ptr);

// MP3 helper function prototypes
llist_elem_t *llist_find_max(llist_elem_t *start, llist_elem_t *end, int (*fcomp)(const data_t*, const data_t *));
void llist_swap_data(llist_elem_t *elem1, llist_elem_t *elem2);
void llist_split(llist_t *list_ptr, llist_t **left, llist_t **right);
void llist_merge(llist_t *list_ptr, llist_t *left, llist_t *right);
int llist_qsort_compare(const void *a, const void *b, void *list_ptr);

//Accesses an element in the list at specific index
data_t * llist_access(llist_t *list_ptr, int pos_index)
{
    llist_elem_t *rover = NULL;

    // debugging function to verify that the structure of the list is valid
    // llist_debug_validate(list_ptr);

    
    if (list_ptr->ll_entry_count == 0) {
        return NULL;  // list is empty
    }
    else if (pos_index == LLPOSITION_FRONT || pos_index == 0) {
        return list_ptr->ll_front->data_ptr;
    }
    else if (pos_index == LLPOSITION_BACK || pos_index == list_ptr->ll_entry_count - 1) {
        return list_ptr->ll_back->data_ptr;
    }
    else if (pos_index < 0 || pos_index >= list_ptr->ll_entry_count)
        return NULL;   // does not correspond to position in list

    // loop through the list until find correct position index
    rover = list_ptr->ll_front;
    for (int i = 0; i < pos_index; i++){
            rover = rover->ll_next;
    }

    // already verified that pos_index should be valid so rover better not be null
    assert(rover != NULL);
    assert(rover->data_ptr != NULL);
    return rover->data_ptr;
}

/* Allocates a new, empty list
 *
 * If the comparison function is NULL, then the list is unsorted.
 *
 * Otherwise, the list is initially assumed to be sorted.  Note that if
 * list_insert is used the list is changed to unsorted.
 *
 * The field sorted can only take values LLIST_SORTED or LLIST_UNSORTED
 *
 * Use llist_destruct to remove and deallocate all elements on a list
 * and the header block.
 *
 * (This function is written and no changes needed. It provides an example
 *  of how save the comparison function pointer.  See other examples in this
 *  file for how to use compare_fun.)
 */
llist_t * llist_construct(int (*fcomp)(const data_t *, const data_t *))
{
    llist_t *new_list;
    new_list = (llist_t *) malloc(sizeof(llist_t));
    new_list->ll_front = NULL;
    new_list->ll_back = NULL;
    new_list->ll_entry_count = 0;
    new_list->compare_fun = fcomp;
    if (fcomp == NULL)
        new_list->ll_sorted_state = LLIST_UNSORTED;
    else
        new_list->ll_sorted_state = LLIST_SORTED;

    // llist_debug_validate(new_list);
    return new_list;
}

//Finds an element in the list and returns a pointer to it
data_t * llist_elem_find(llist_t *list_ptr, data_t *elem_ptr, int *pos_index,
        int (*fcomp)(const data_t *, const data_t *))
{
    // llist_debug_validate(list_ptr);

    //Handle empty list
    if (list_ptr->ll_entry_count == 0) {
            return NULL;
    }
    //Search through list
    llist_elem_t *current = list_ptr->ll_front;
    int position = 0;

    while (current != NULL) {
            //Check if elements match
            if (fcomp(elem_ptr, current->data_ptr) == 0) {
                    *pos_index = position;
                    return current->data_ptr;
            }
            current = current->ll_next;
            position++;
    }
    //Element not found
    return NULL;
}

//Deallocates the contents of the list
void llist_destruct(llist_t *list_ptr)
{
    // the first line must validate the list
    // llist_debug_validate(list_ptr);

    //Free all elements in list
    llist_elem_t *current = list_ptr->ll_front;
    while(current != NULL) {
            llist_elem_t *next = current->ll_next;
            free(current->data_ptr);
            free(current);
            current = next;
    }
    //Free the list header
    free(list_ptr);
}



//Inserts an element into the list at a specified index
void llist_insert(llist_t *list_ptr, data_t *elem_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    assert(pos_index == LLPOSITION_FRONT || pos_index == LLPOSITION_BACK || pos_index >= 0);


    //Handle special position values
    if (pos_index == LLPOSITION_FRONT) {
            pos_index = 0;
    } else if(pos_index == LLPOSITION_BACK) {
            pos_index = list_ptr->ll_entry_count;
    }

    // If pos_index is beyond the end, append to the end
    if (pos_index > list_ptr->ll_entry_count) {
            pos_index = list_ptr->ll_entry_count;
    }

    //Create new element
    llist_elem_t *new_elem = (llist_elem_t *)malloc(sizeof(llist_elem_t));
    if (new_elem == NULL) {
            return; //memory allocation failed
    }
    new_elem->data_ptr = elem_ptr;
    new_elem->ll_prev = NULL;
    new_elem->ll_next = NULL;

    //Handle empty list
    if (list_ptr->ll_entry_count == 0) {
            list_ptr->ll_front = new_elem;
            list_ptr->ll_back = new_elem;
    }
    //Insert at front
    else if (pos_index == 0) {
            new_elem->ll_next = list_ptr->ll_front;
            list_ptr->ll_front->ll_prev = new_elem;
            list_ptr->ll_front = new_elem;
    }
    //Insert at back
    else if (pos_index == list_ptr->ll_entry_count) {
            new_elem->ll_prev = list_ptr->ll_back;
            list_ptr->ll_back->ll_next = new_elem;
            list_ptr->ll_back = new_elem;
    }

    //Insert in middle
    else {
            llist_elem_t *current = list_ptr->ll_front;
            for (int i = 0; i < pos_index; i++) {
                    current = current->ll_next;
            }

            new_elem->ll_prev = current->ll_prev;
            new_elem->ll_next = current;
            current->ll_prev->ll_next = new_elem;
            current->ll_prev = new_elem;
    }

    list_ptr->ll_entry_count++;






    // the last three lines of this function must be the following
    if (list_ptr->ll_sorted_state == LLIST_SORTED)
        list_ptr->ll_sorted_state = LLIST_UNSORTED;
    // llist_debug_validate(list_ptr);
}

//Inserts an element into the list at the correct position
void llist_insert_sorted(llist_t *list_ptr, data_t *elem_ptr)
{
    assert(list_ptr != NULL);
    if (list_ptr-> ll_sorted_state != LLIST_SORTED ) {
            printf("Warning: llist_insert_sorted called on unsorted list, converting to sorted\n");
            list_ptr->ll_sorted_state = LLIST_SORTED;
    }
    //Handle empty list
    if (list_ptr->ll_entry_count == 0) {
            llist_elem_t *new_elem = (llist_elem_t *)malloc(sizeof(llist_elem_t));
            new_elem->data_ptr = elem_ptr;
            new_elem->ll_prev = NULL;
            new_elem->ll_next = NULL;
            list_ptr->ll_front = new_elem;
            list_ptr->ll_back = new_elem;
            list_ptr->ll_entry_count++;
            // llist_debug_validate(list_ptr);
            return;
    }
    //Find the correct position to insert
    llist_elem_t *current = list_ptr->ll_front;
    int position = 0;

    while (current != NULL && list_ptr->compare_fun(elem_ptr, current->data_ptr) <= 0) {
            current = current->ll_next;
            position++;
    }
    //Insert at found position using direct insertion to preserve sorted state
    llist_elem_t *new_elem = (llist_elem_t *)malloc(sizeof(llist_elem_t));
    if (new_elem == NULL) {
            return; //memory allocation failed
    }
    new_elem->data_ptr = elem_ptr;
    new_elem->ll_prev = NULL;
    new_elem->ll_next = NULL;

    //Handle insertion at front
    if (position == 0) {
            new_elem->ll_next = list_ptr->ll_front;
            if (list_ptr->ll_front != NULL) {
                    list_ptr->ll_front->ll_prev = new_elem;
            }
            list_ptr->ll_front = new_elem;
            if (list_ptr->ll_entry_count == 0) {
                    list_ptr->ll_back = new_elem;
            }
    }
    //Handle insertion at back
    else if (position == list_ptr->ll_entry_count) {
            new_elem->ll_prev = list_ptr->ll_back;
            if (list_ptr->ll_back != NULL) {
                    list_ptr->ll_back->ll_next = new_elem;
            }
            list_ptr->ll_back = new_elem;
    }
    //Handle insertion in middle
    else {
            llist_elem_t *rover = list_ptr->ll_front;
            for (int i = 0; i < position; i++) {
                    rover = rover->ll_next;
            }
            new_elem->ll_prev = rover->ll_prev;
            new_elem->ll_next = rover;
            if (rover->ll_prev != NULL) {
                    rover->ll_prev->ll_next = new_elem;
            }
            rover->ll_prev = new_elem;
    }

    list_ptr->ll_entry_count++;




    // the last line checks if the new list is correct
    // llist_debug_validate(list_ptr);
}

//Removes an element from the list at specific index
data_t * llist_remove(llist_t *list_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    assert(pos_index == LLPOSITION_FRONT || pos_index == LLPOSITION_BACK || pos_index >= 0);
    // note you MUST handle the case that pos_index is LLPOSITION_FRONT or BACK

    if (pos_index == LLPOSITION_FRONT) {
            pos_index = 0;
    } else if (pos_index == LLPOSITION_BACK) {
            pos_index = list_ptr->ll_entry_count - 1;
    }

    //Handle empty list or invalid position
    if (list_ptr->ll_entry_count == 0 || pos_index < 0 || pos_index >= list_ptr->ll_entry_count) {
           return NULL;
    }
    llist_elem_t *rover = list_ptr->ll_front;
    for (int i = 0; i < pos_index; i++) {
            rover = rover->ll_next;
    }
    //Element to be remover is rover
    data_t *removed_data = rover->data_ptr;

    if (rover->ll_prev != NULL) {
            rover->ll_prev->ll_next = rover->ll_next;
    } else {
            //Removing the front element
            list_ptr->ll_front = rover->ll_next;
    }

    if (rover->ll_next != NULL) {
            rover->ll_next->ll_prev = rover->ll_prev;
    } else {
            //Removing the back element
            list_ptr->ll_back = rover->ll_prev;
    }

    free(rover);
    list_ptr->ll_entry_count--;




    // the last line should verify the list is valid after the remove
    // llist_debug_validate(list_ptr);
    return removed_data;

}

//Returns the number of elements in the list
int llist_entries(llist_t *list_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->ll_entry_count >= 0);
    return list_ptr->ll_entry_count;
}


/* This function verifies that the pointers for the two-way linked list are
 * valid, and that the list size matches the number of items in the list.
 *
 * If the linked list is sorted it also checks that the elements in the list
 * appear in the proper order.
 *
 * The function produces no output if the two-way linked list is correct.  It
 * causes the program to terminate and print a line beginning with "Assertion
 * failed:" if an error is detected.
 *
 * The checks are not exhaustive, so an error may still exist in the
 * list even if these checks pass.
 *
 * YOU MUST NOT CHANGE THIS FUNCTION.  WE USE IT DURING GRADING TO VERIFY THAT
 * YOUR LIST IS CONSISTENT.
 */
void llist_debug_validate(llist_t *L)
{
    llist_elem_t *N;
    int count = 0;
    assert(L != NULL);
    if (L->ll_front == NULL)
        assert(L->ll_back == NULL && L->ll_entry_count == 0);
    if (L->ll_back == NULL)
        assert(L->ll_front == NULL && L->ll_entry_count == 0);
    if (L->ll_entry_count == 0)
        assert(L->ll_front == NULL && L->ll_back == NULL);
    if (L->ll_entry_count == 1) {
        assert(L->ll_front == L->ll_back && L->ll_front != NULL);
        assert(L->ll_front->ll_next == NULL && L->ll_front->ll_prev == NULL);
        assert(L->ll_front->data_ptr != NULL);
    }
    if (L->ll_front == L->ll_back && L->ll_front != NULL)
        assert(L->ll_entry_count == 1);
    assert(L->ll_sorted_state == LLIST_SORTED || L->ll_sorted_state == LLIST_UNSORTED);
    if (L->ll_entry_count > 1) {
        assert(L->ll_front != L->ll_back && L->ll_front != NULL && L->ll_back != NULL);
        N = L->ll_front;
        assert(N->ll_prev == NULL);
        while (N != NULL) {
            assert(N->data_ptr != NULL);
            if (N->ll_next != NULL) assert(N->ll_next->ll_prev == N);
            else assert(N == L->ll_back);
            count++;
            N = N->ll_next;
        }
        assert(count == L->ll_entry_count);
    }
    if (L->ll_sorted_state == LLIST_SORTED && L->ll_front != NULL) {
        N = L->ll_front;
        while (N->ll_next != NULL) {
            assert(L->compare_fun(N->data_ptr, N->ll_next->data_ptr) != -1);   // A <= B
            N = N->ll_next;
        }
    }
}
/* MP3: Main sorting function that routes to different sorting algorithms
 * sort_type: 1=insertion, 2=recursive selection, 3=iterative selection, 4=merge, 5=quick
 * fcomp: comparison function pointer
 */
void llist_sort(llist_t *list_ptr, int sort_type, int (*fcomp)(const data_t*, const data_t *))
{
    assert(list_ptr != NULL);
    assert(fcomp != NULL);
    
    // Update the comparison function in the list header
    list_ptr->compare_fun = fcomp;
    
    // Handle empty or single element lists
    if (list_ptr->ll_entry_count <= 1) {
        list_ptr->ll_sorted_state = LLIST_SORTED;
        llist_debug_validate(list_ptr);
        return;
    }
    
    // Route to appropriate sorting algorithm
    switch (sort_type) {
        case 1: // Insertion sort
            llist_insertion_sort(list_ptr);
            break;
        case 2: // Recursive selection sort
            llist_recursive_selection_sort(list_ptr);
            break;
        case 3: // Iterative selection sort
            llist_iterative_selection_sort(list_ptr);
            break;
        case 4: // Merge sort
            llist_merge_sort(list_ptr);
            break;
        case 5: // Quick sort
            llist_quick_sort(list_ptr);
            break;
        default:
            printf("Error: Invalid sort type %d\n", sort_type);
            return;
    }
    
    // Required final lines
    list_ptr->ll_sorted_state = LLIST_SORTED;
    llist_debug_validate(list_ptr);
}

/* MP3: Insertion sort - uses existing llist_remove and llist_insert_sorted */
void llist_insertion_sort(llist_t *list_ptr)
{
    llist_t *temp_list = llist_construct(list_ptr->compare_fun);
    
    // Remove elements from original list and insert into sorted temp list
    while (list_ptr->ll_entry_count > 0) {
        data_t *elem = llist_remove(list_ptr, LLPOSITION_FRONT);
        llist_insert_sorted(temp_list, elem);
    }
    
    // Move elements back to original list
    while (temp_list->ll_entry_count > 0) {
        data_t *elem = llist_remove(temp_list, LLPOSITION_FRONT);
        llist_insert(list_ptr, elem, LLPOSITION_BACK);
    }
    
    llist_destruct(temp_list);
}

/* MP3: Helper function to find maximum element in range - follows Program 5.20 exactly */
llist_elem_t *llist_find_max(llist_elem_t *start, llist_elem_t *end, int (*fcomp)(const data_t*, const data_t *))
{
    llist_elem_t *i = start;  // i is an index that visits all positions from m to n
    llist_elem_t *j = start;  // j is an index that saves the position of the largest
    
    // do-while loop exactly as in Program 5.20
    do {
        i = i->ll_next;  // advance i to point to next number A[i]
        if (i != end && fcomp(i->data_ptr, j->data_ptr) > 0) {  // if A[i] > A[j]
            j = i;  // save the position, i, of the largest in j
        }
    } while (i != end);  // stop when all i in m:n have been tested
    
    return j;  // return j == position of the largest number A[j] in A[m:n]
}

/* MP3: Helper function to swap data between two elements */
void llist_swap_data(llist_elem_t *elem1, llist_elem_t *elem2)
{
    data_t *temp = elem1->data_ptr;
    elem1->data_ptr = elem2->data_ptr;
    elem2->data_ptr = temp;
}

/* MP3: Recursive selection sort - follows Program 5.19 exactly */
void llist_recursive_selection_sort(llist_t *list_ptr)
{
    if (list_ptr->ll_entry_count <= 1) return;
    
    // Call helper function with front and back elements
    llist_recursive_selection_sort_helper(list_ptr, list_ptr->ll_front, list_ptr->ll_back);
}

/* Helper function that follows Program 5.19 logic exactly */
void llist_recursive_selection_sort_helper(llist_t *list_ptr, llist_elem_t *m, llist_elem_t *n)
{
    llist_elem_t *max_position;
    data_t *temp;
    
    // if (m < n) - check if there's more than one element
    if (m != n) {
        // Let MaxPosition be the index of the largest number in A[m:n]
        max_position = llist_find_max(m, n->ll_next, list_ptr->compare_fun);
        
        // Exchange A[m] <-> A[MaxPosition]
        temp = m->data_ptr;
        m->data_ptr = max_position->data_ptr;
        max_position->data_ptr = temp;
        
        // SelectionSort the subarray A[m+1:n]
        if (m->ll_next != NULL) {
            llist_recursive_selection_sort_helper(list_ptr, m->ll_next, n);
        }
    }
}

/* MP3: Iterative selection sort - follows Program 5.35 exactly */
void llist_iterative_selection_sort(llist_t *list_ptr)
{
    llist_elem_t *m = list_ptr->ll_front;  // m is the boundary between sorted and unsorted
    llist_elem_t *n = list_ptr->ll_back;   // n is the end of the list
    
    llist_elem_t *max_position, *i;
    data_t *temp;
    
    // while (m < n) - while there are unsorted elements
    while (m != n) {
        // i = m; MaxPosition = m;
        i = m;
        max_position = m;
        
        // do-while loop exactly as in Program 5.35
        do {
            i = i->ll_next;  // i++;
            if (i != NULL && list_ptr->compare_fun(i->data_ptr, max_position->data_ptr) > 0) {  // if (A[i] > A[MaxPosition])
                max_position = i;  // MaxPosition = i;
            }
        } while (i != n);  // while (i != n);
        
        // temp = A[m]; A[m] = A[MaxPosition]; A[MaxPosition] = temp;
        temp = m->data_ptr;
        m->data_ptr = max_position->data_ptr;
        max_position->data_ptr = temp;
        
        // m++;
        m = m->ll_next;
    }
}

/* MP3: Helper function to split list into two halves */
void llist_split(llist_t *list_ptr, llist_t **left, llist_t **right)
{
    int size = list_ptr->ll_entry_count;
    int mid = size / 2;
    
    *left = llist_construct(list_ptr->compare_fun);
    *right = llist_construct(list_ptr->compare_fun);
    
    // Move first half to left list
    for (int i = 0; i < mid; i++) {
        data_t *elem = llist_remove(list_ptr, LLPOSITION_FRONT);
        llist_insert(*left, elem, LLPOSITION_BACK);
    }
    
    // Move remaining elements to right list
    while (list_ptr->ll_entry_count > 0) {
        data_t *elem = llist_remove(list_ptr, LLPOSITION_FRONT);
        llist_insert(*right, elem, LLPOSITION_BACK);
    }
}

/* MP3: Helper function to merge two sorted lists */
void llist_merge(llist_t *list_ptr, llist_t *left, llist_t *right)
{
    while (left->ll_entry_count > 0 && right->ll_entry_count > 0) {
        data_t *left_elem = llist_access(left, 0);
        data_t *right_elem = llist_access(right, 0);
        
        if (list_ptr->compare_fun(left_elem, right_elem) >= 0) {
            data_t *elem = llist_remove(left, LLPOSITION_FRONT);
            llist_insert(list_ptr, elem, LLPOSITION_BACK);
        } else {
            data_t *elem = llist_remove(right, LLPOSITION_FRONT);
            llist_insert(list_ptr, elem, LLPOSITION_BACK);
        }
    }
    
    // Add remaining elements from left list
    while (left->ll_entry_count > 0) {
        data_t *elem = llist_remove(left, LLPOSITION_FRONT);
        llist_insert(list_ptr, elem, LLPOSITION_BACK);
    }
    
    // Add remaining elements from right list
    while (right->ll_entry_count > 0) {
        data_t *elem = llist_remove(right, LLPOSITION_FRONT);
        llist_insert(list_ptr, elem, LLPOSITION_BACK);
    }
}

/* MP3: Merge sort - follows Program 6.19 strategy exactly */
void llist_merge_sort(llist_t *list_ptr)
{
    // if (the List has more than one item in it)
    if (list_ptr->ll_entry_count > 1) {
        llist_t *left, *right;
        
        // (break the List into two half-lists, L = LeftList and R = RightList)
        llist_split(list_ptr, &left, &right);
        
        // (sort the LeftList using MergeSort(L))
        llist_merge_sort(left);
        
        // (sort the RightList using MergeSort(R))
        llist_merge_sort(right);
        
        // (merge L and R into a single sorted List)
        llist_merge(list_ptr, left, right);
        
        llist_destruct(left);
        llist_destruct(right);
    }
    // else { (do nothing, since the list is already sorted) }
}

/* MP3: Quick sort using qsort_r */
void llist_quick_sort(llist_t *list_ptr)
{
    int size = llist_entries(list_ptr);
    if (size <= 1) return;
    
    // Create array of pointers to data
    data_t **array = (data_t **) malloc(size * sizeof(data_t *));
    
    // Fill array with data from list
    for (int i = 0; i < size; i++) {
        array[i] = llist_remove(list_ptr, LLPOSITION_FRONT);
    }
    
    // Sort array using qsort_r
    qsort_r(array, size, sizeof(data_t *), llist_qsort_compare, list_ptr);
    
    // Put sorted data back into list
    for (int i = 0; i < size; i++) {
        llist_insert(list_ptr, array[i], LLPOSITION_BACK);
    }
    
    free(array);
}

/* MP3: Comparison function for qsort_r */
int llist_qsort_compare(const void *a, const void *b, void *list_ptr)
{
    llist_t *list = (llist_t *) list_ptr;
    data_t *data_a = *(data_t **) a;
    data_t *data_b = *(data_t **) b;
    return list->compare_fun(data_b, data_a); // Note: reversed for descending order
}

/* commands for vim. ts: tabstop, sts: softtabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */