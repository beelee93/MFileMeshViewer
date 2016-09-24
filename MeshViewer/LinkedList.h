#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

template<typename T> struct ListNode {
	int id;
	T item;
	ListNode* next;
};

template<typename value_type> class LinkedList {
public:
	LinkedList() {
		listHead = NULL;
		size = 0;
		nextId = 0;
	}

	~LinkedList() {
		ListNode<value_type> *cur, *temp;
		cur = listHead;
		while (cur) {
			temp = cur->next;
			delete cur;
			cur = temp;
		}
	}

	int getSize() {
		return size;
	}

	ListNode<value_type>* put(const value_type& item) {
		ListNode<value_type>* newItem;
		newItem = new ListNode < value_type >;
		newItem->item = item;
		newItem->next = NULL;
		newItem->id = nextId++;

		if (!listHead) {
			listHead = newItem;
		}
		else {
			// place at front
			newItem->next = listHead;
			listHead = newItem;
		}
		size++;

		return newItem;
	}

	ListNode<value_type>* getHead() {
		return listHead;
	}

	value_type* get(int i) {
		ListNode<value_type> *cur;
		cur = listHead;
		while (cur) {
			if (cur->id == i)
				return &(cur->item);
			cur = cur->next;
		}
		return NULL;
	}
private:
	ListNode<value_type>* listHead;
	int size;
	int nextId;
};
#endif
