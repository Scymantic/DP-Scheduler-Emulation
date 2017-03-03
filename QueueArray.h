

//Adrian Barberis
//Jan 24, 2017
// Queue Class and QueueArray Class

// Last Edit:  Wed, FEB 22, 2017

/*----------------------------- PREPROCESSORS -----------------------------*/


#ifndef QUEUEARRAY_H
#define QUEUEARRAY_H

#include <vector>
#include <iostream>
using namespace::std;



/*----------------------------- DEFINITIONS -----------------------------*/

//Queue class

template< class K> class Queue
{
	public:
		Queue(){};
		~Queue(){};

		void push(K item)
		{
			//  Add item to vector then set size equal to the size of the vector
			queue.push_back(item);
			_size = queue.size();
		};

		K pop()
		{
			//Swap first and last elements;
			K front = queue[0];
			K back = queue[queue.size()-1];
			queue[0] = back;
			queue[queue.size()-1] = front;

			//Remove last element a.k.a first element; 
			//in other words the first element put into the queue.
			queue.pop_back();
			_size = queue.size();
			
			//put last element back into queue at end
			//delete second copy of last element which is currently at queue[0]
			queue.push_back(back);
			queue.erase(queue.begin());
			return front;
		};

		const int getSize()
		{
			// get the size of the queue (vector)
			return _size;	
		};

		const K get(int index)
		{
			//  get a the value at location "index" of the queue (vector)
			return queue.at(index);
		}

		const bool isEmpty()
		{
			//  check if the queue is empty

			// 	returns TRUE ONLY IF the queue IS empty
			if(!queue.empty())
			{
				return false;
			}
			return true;
		};

	private:
		vector<K> queue;
		int _size;
};



//==================================================================================================||






//Array of queues


template<class T> class QueueArray
{
	public:
		QueueArray(int size)
		{
			//Initialize array as an array of size n of Queues
			_array = new Queue<T>[size];

			//  If _array is nullptr then we could not initialize the QueueArray
			if(_array == nullptr)
			{
				cout << "Array initialization not possible: Not enough free memory" << endl;
				exit(-1);

			}
			for(int i=0; i<size; i++)
			{
				//  make a queue and store it at _arry[i];
				Queue<T> q;
				_array[i] = q;
			}
			_size = size;
		};

		~QueueArray()
		{
			// destructor
			delete[]_array;
		};

		int Enqueue(T item, int index)
		{
			// if index is less than zero or greater than size quit
			if(index > _size || index < 0)
			{
				return -1;
			}

			//Used to check if queue changed size
			//If it did then the addition was successful. (Assuming the new size is > than the old one)
			int sizepre = _array[index].getSize();

			//  add item to queue at the specified index
			_array[index].push(item);

			//  check for successful Enqueue
			if(sizepre < _array[index].getSize())
			{
				return 1;
			}
			else
			{
				return 0;
			}
		};

		T Dequeue()
		{
			//  index to used later to "get" the item
			int k = 0;

			//  boolean flag for empty
			bool allEmpty = false;

			//Find first nonempty
			for(int i = 0; i<_size; i++)
			{
				if(_array[i].isEmpty() == false)
				{
					// set K equal to array index of first non empty
					k = i;

					//  set boolean flag to false
					allEmpty = false;

					//exit loop
					break;
				}
				allEmpty = true;
			}	

			if(allEmpty)
			{
				return 0;
			}
			
			return _array[k].pop();
			
		};
	
		const int Qsize(int index)
		{
			//  Return the size of a queue at the given index
			if(index > _size || index < 0)
			{
				//  if index is greater than the _array size or less than 0 quit
				return -1;
			}
			else
			{
				return _array[index].getSize();
			}
		};

		const int Asize()
		{
			//  Return the size of _array
			return _size;
		};

		const int QAsize()
		{
			//  return the total size of all the queues in the QueueArray
			int total = 0;
			for(int i = 0; i<_size; i++)
			{
				total += _array[i].getSize();
			}
			return total;
		};

		T* Qstate(int index)
		{

			//  Return an array that is a copy of the queue at location "index"
			if(index > _size || index < 0)
			{
				return nullptr;
			}
			
			T *arr = new T[_array[index].getSize()];
			for(int i=0; i<_array[index].getSize(); i++)
			{
				arr[i] = _array[index].get(i);
			}
			return arr;
		};

		const bool isEmpty()
		{
			//  check if all of the queues are empty
			if(QAsize() > 0)
			{
				return false;
			}
			return true;
		};
			

	private:
		Queue<T> *_array;
		int _size;

};

#endif /* QUEUEARRAY_H */



